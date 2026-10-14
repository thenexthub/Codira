//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - `codira_gpu`: the host side of `@target(gpu)` -- loading the PTX that
//!   `codira_codegen` compiles kernels to, moving buffers to and from the
//!   device, and launching kernels, through the CUDA driver API.
//!
//! # Why the driver API, loaded at run time
//!
//! The driver API (`cuModuleLoadData`, `cuLaunchKernel`) takes PTX text and
//! JIT-compiles it for whatever GPU is present, so a kernel compiled once by
//! LLVM's NVPTX backend runs on any NVIDIA GPU new enough for its PTX
//! version. The library (`nvcuda.dll`, `libcuda.so.1`) is part of the
//! display driver, not of the CUDA toolkit, so it is opened with
//! `libloading` when a [`Gpu`] is created: a Codira build links no CUDA
//! library and runs on machines without a GPU, and asking for one there is
//! an ordinary error ([`GpuError::NoDriver`], [`GpuError::NoDevice`]).
//!
//! # Safety
//!
//! A kernel reads and writes device memory through the addresses it is
//! given; nothing checks them, as in CUDA C. [`Gpu::launch`] is therefore
//! `unsafe`. Device buffers ([`DeviceBuffer`]) are owned and freed on drop,
//! and every copy is bounds-checked on the host side.

use std::{
    ffi::{c_char, c_int, c_uint, c_void, CStr, CString},
    sync::Arc,
};

type CuResult = c_int;
type CuDevice = c_int;
type CuContext = *mut c_void;
type CuModule = *mut c_void;
type CuFunction = *mut c_void;
type CuDevicePtr = u64;

const CUDA_SUCCESS: CuResult = 0;
const CUDA_ERROR_NO_DEVICE: CuResult = 100;
/// `CU_JIT_ERROR_LOG_BUFFER` / `_SIZE_BYTES`: where the PTX JIT writes why it
/// rejected a module.
const CU_JIT_ERROR_LOG_BUFFER: c_int = 5;
const CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES: c_int = 6;

/// Why the GPU could not be used.
#[derive(Debug, Clone, PartialEq, Eq, thiserror::Error)]
pub enum GpuError {
    #[error("no CUDA driver is installed ({0})")]
    NoDriver(String),
    #[error("the CUDA driver found no GPU")]
    NoDevice,
    #[error("{call} failed: {message} (CUresult {code})")]
    Driver {
        call: &'static str,
        code: i32,
        message: String,
    },
    #[error("the PTX JIT rejected the module: {0}")]
    BadModule(String),
    #[error("the module has no kernel named `{0}`")]
    NoKernel(String),
    #[error("a copy of {requested} bytes does not fit a buffer of {available}")]
    OutOfBounds { requested: usize, available: usize },
}

/// The driver API entry points this crate uses, resolved from the library.
struct Api {
    _library: libloading::Library,
    cu_init: unsafe extern "system" fn(c_uint) -> CuResult,
    cu_device_get_count: unsafe extern "system" fn(*mut c_int) -> CuResult,
    cu_device_get: unsafe extern "system" fn(*mut CuDevice, c_int) -> CuResult,
    cu_device_get_name: unsafe extern "system" fn(*mut c_char, c_int, CuDevice) -> CuResult,
    cu_ctx_create: unsafe extern "system" fn(*mut CuContext, c_uint, CuDevice) -> CuResult,
    cu_ctx_destroy: unsafe extern "system" fn(CuContext) -> CuResult,
    cu_ctx_set_current: unsafe extern "system" fn(CuContext) -> CuResult,
    cu_ctx_synchronize: unsafe extern "system" fn() -> CuResult,
    cu_module_load_data_ex: unsafe extern "system" fn(
        *mut CuModule,
        *const c_void,
        c_uint,
        *mut c_int,
        *mut *mut c_void,
    ) -> CuResult,
    cu_module_unload: unsafe extern "system" fn(CuModule) -> CuResult,
    cu_module_get_function:
        unsafe extern "system" fn(*mut CuFunction, CuModule, *const c_char) -> CuResult,
    cu_mem_alloc: unsafe extern "system" fn(*mut CuDevicePtr, usize) -> CuResult,
    cu_mem_free: unsafe extern "system" fn(CuDevicePtr) -> CuResult,
    cu_memcpy_htod: unsafe extern "system" fn(CuDevicePtr, *const c_void, usize) -> CuResult,
    cu_memcpy_dtoh: unsafe extern "system" fn(*mut c_void, CuDevicePtr, usize) -> CuResult,
    #[allow(clippy::type_complexity)]
    cu_launch_kernel: unsafe extern "system" fn(
        CuFunction,
        c_uint,
        c_uint,
        c_uint,
        c_uint,
        c_uint,
        c_uint,
        c_uint,
        *mut c_void,
        *mut *mut c_void,
        *mut *mut c_void,
    ) -> CuResult,
    cu_get_error_string: unsafe extern "system" fn(CuResult, *mut *const c_char) -> CuResult,
}

#[cfg(windows)]
const LIBRARY: &str = "nvcuda.dll";
#[cfg(not(windows))]
const LIBRARY: &str = "libcuda.so.1";

impl Api {
    fn load() -> Result<Api, GpuError> {
        // SAFETY: loading the CUDA driver runs its initializers, which is
        // what it is for; the symbols below are resolved with the C
        // signatures the driver API documents.
        unsafe {
            let library =
                libloading::Library::new(LIBRARY).map_err(|e| GpuError::NoDriver(e.to_string()))?;
            macro_rules! sym {
                ($name:literal) => {
                    *library
                        .get(concat!($name, "\0").as_bytes())
                        .map_err(|e| GpuError::NoDriver(format!("{}: {e}", $name)))?
                };
            }
            Ok(Api {
                cu_init: sym!("cuInit"),
                cu_device_get_count: sym!("cuDeviceGetCount"),
                cu_device_get: sym!("cuDeviceGet"),
                cu_device_get_name: sym!("cuDeviceGetName"),
                cu_ctx_create: sym!("cuCtxCreate_v2"),
                cu_ctx_destroy: sym!("cuCtxDestroy_v2"),
                cu_ctx_set_current: sym!("cuCtxSetCurrent"),
                cu_ctx_synchronize: sym!("cuCtxSynchronize"),
                cu_module_load_data_ex: sym!("cuModuleLoadDataEx"),
                cu_module_unload: sym!("cuModuleUnload"),
                cu_module_get_function: sym!("cuModuleGetFunction"),
                cu_mem_alloc: sym!("cuMemAlloc_v2"),
                cu_mem_free: sym!("cuMemFree_v2"),
                cu_memcpy_htod: sym!("cuMemcpyHtoD_v2"),
                cu_memcpy_dtoh: sym!("cuMemcpyDtoH_v2"),
                cu_launch_kernel: sym!("cuLaunchKernel"),
                cu_get_error_string: sym!("cuGetErrorString"),
                _library: library,
            })
        }
    }

    fn check(&self, call: &'static str, code: CuResult) -> Result<(), GpuError> {
        if code == CUDA_SUCCESS {
            return Ok(());
        }
        let mut text: *const c_char = std::ptr::null();
        // SAFETY: cuGetErrorString writes a pointer to a static string.
        let message = unsafe {
            if (self.cu_get_error_string)(code, &mut text) == CUDA_SUCCESS && !text.is_null() {
                CStr::from_ptr(text).to_string_lossy().into_owned()
            } else {
                "unknown error".to_owned()
            }
        };
        Err(GpuError::Driver {
            call,
            code,
            message,
        })
    }
}

struct Context {
    api: Api,
    context: CuContext,
}

// SAFETY: a CUDA context may be made current on any thread
// (`cuCtxSetCurrent`), which every operation here does first.
unsafe impl Send for Context {}
unsafe impl Sync for Context {}

impl Drop for Context {
    fn drop(&mut self) {
        // SAFETY: the context was created by this value and is destroyed once.
        unsafe {
            (self.api.cu_ctx_destroy)(self.context);
        }
    }
}

/// A CUDA device with a context on it. Cheap to clone; the context lives as
/// long as any clone, module or buffer does.
#[derive(Clone)]
pub struct Gpu {
    inner: Arc<Context>,
    name: String,
}

impl Gpu {
    /// Opens the first GPU the driver reports.
    pub fn open() -> Result<Gpu, GpuError> {
        let api = Api::load()?;
        // SAFETY: plain driver API calls with valid out-pointers.
        unsafe {
            let init = (api.cu_init)(0);
            if init == CUDA_ERROR_NO_DEVICE {
                return Err(GpuError::NoDevice);
            }
            api.check("cuInit", init)?;
            let mut count = 0;
            api.check("cuDeviceGetCount", (api.cu_device_get_count)(&mut count))?;
            if count == 0 {
                return Err(GpuError::NoDevice);
            }
            let mut device = 0;
            api.check("cuDeviceGet", (api.cu_device_get)(&mut device, 0))?;
            let mut name = [0 as c_char; 256];
            api.check(
                "cuDeviceGetName",
                (api.cu_device_get_name)(name.as_mut_ptr(), name.len() as c_int, device),
            )?;
            let name = CStr::from_ptr(name.as_ptr()).to_string_lossy().into_owned();
            let mut context = std::ptr::null_mut();
            api.check("cuCtxCreate", (api.cu_ctx_create)(&mut context, 0, device))?;
            Ok(Gpu {
                inner: Arc::new(Context { api, context }),
                name,
            })
        }
    }

    /// The device's name, as the driver reports it.
    pub fn name(&self) -> &str {
        &self.name
    }

    fn current(&self) -> Result<&Api, GpuError> {
        let api = &self.inner.api;
        // SAFETY: the context is alive for as long as `self` is.
        api.check("cuCtxSetCurrent", unsafe {
            (api.cu_ctx_set_current)(self.inner.context)
        })?;
        Ok(api)
    }

    /// Loads PTX text (as `codira_codegen` produces it) and JIT-compiles it
    /// for this device.
    pub fn load_ptx(&self, ptx: &str) -> Result<Module, GpuError> {
        let api = self.current()?;
        let source = CString::new(ptx).map_err(|e| GpuError::BadModule(e.to_string()))?;
        let mut log = vec![0u8; 8192];
        let mut options = [CU_JIT_ERROR_LOG_BUFFER, CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES];
        let mut values = [log.as_mut_ptr().cast::<c_void>(), log.len() as *mut c_void];
        let mut module = std::ptr::null_mut();
        // SAFETY: `source` is NUL-terminated PTX; the option arrays have the
        // length passed and outlive the call.
        let result = unsafe {
            (api.cu_module_load_data_ex)(
                &mut module,
                source.as_ptr().cast(),
                options.len() as c_uint,
                options.as_mut_ptr(),
                values.as_mut_ptr(),
            )
        };
        if result != CUDA_SUCCESS {
            let end = log.iter().position(|&b| b == 0).unwrap_or(log.len());
            let detail = String::from_utf8_lossy(&log[..end]).trim().to_owned();
            return Err(match api.check("cuModuleLoadDataEx", result) {
                Err(error) if detail.is_empty() => error,
                _ => GpuError::BadModule(detail),
            });
        }
        Ok(Module {
            gpu: self.clone(),
            module,
        })
    }

    /// Allocates `bytes` of device memory.
    pub fn alloc(&self, bytes: usize) -> Result<DeviceBuffer, GpuError> {
        let api = self.current()?;
        let mut address = 0;
        // SAFETY: valid out-pointer; a zero-byte request is rounded up so the
        // buffer always has an address.
        api.check("cuMemAlloc", unsafe {
            (api.cu_mem_alloc)(&mut address, bytes.max(1))
        })?;
        Ok(DeviceBuffer {
            gpu: self.clone(),
            address,
            bytes,
        })
    }

    /// Allocates a device buffer holding a copy of `data`.
    pub fn upload<T: Copy>(&self, data: &[T]) -> Result<DeviceBuffer, GpuError> {
        let buffer = self.alloc(std::mem::size_of_val(data))?;
        buffer.copy_from(data)?;
        Ok(buffer)
    }

    /// Launches `kernel` on a `grid` of `block`-sized thread blocks (x only)
    /// and waits for it to finish.
    ///
    /// # Safety
    ///
    /// `args` must match the kernel's parameters in number, order and type,
    /// and every address passed must be valid device memory for what the
    /// kernel does with it. Nothing on the host can check either.
    pub unsafe fn launch(
        &self,
        kernel: &Kernel<'_>,
        grid: u32,
        block: u32,
        args: &[KernelArg],
    ) -> Result<(), GpuError> {
        let api = self.current()?;
        let mut storage: Vec<u64> = args.iter().map(KernelArg::bits).collect();
        let mut pointers: Vec<*mut c_void> = storage
            .iter_mut()
            .map(|slot| (slot as *mut u64).cast::<c_void>())
            .collect();
        api.check(
            "cuLaunchKernel",
            (api.cu_launch_kernel)(
                kernel.function,
                grid,
                1,
                1,
                block,
                1,
                1,
                0,
                std::ptr::null_mut(),
                pointers.as_mut_ptr(),
                std::ptr::null_mut(),
            ),
        )?;
        api.check("cuCtxSynchronize", (api.cu_ctx_synchronize)())
    }
}

/// A kernel argument. Each is passed by value in the width the kernel
/// declares; little-endian layout means the low bytes of the slot are the
/// value, as the driver reads them.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum KernelArg {
    U32(u32),
    I32(i32),
    U64(u64),
    I64(i64),
    F32(f32),
    F64(f64),
    /// A device buffer's address.
    Buffer(u64),
}

impl KernelArg {
    fn bits(&self) -> u64 {
        match *self {
            KernelArg::U32(v) => u64::from(v),
            KernelArg::I32(v) => u64::from(v as u32),
            KernelArg::U64(v) | KernelArg::Buffer(v) => v,
            KernelArg::I64(v) => v as u64,
            KernelArg::F32(v) => u64::from(v.to_bits()),
            KernelArg::F64(v) => v.to_bits(),
        }
    }
}

/// A loaded PTX module.
pub struct Module {
    gpu: Gpu,
    module: CuModule,
}

impl Module {
    /// The kernel named `name` (a `@target(gpu)` function's name).
    pub fn kernel(&self, name: &str) -> Result<Kernel<'_>, GpuError> {
        let api = self.gpu.current()?;
        let c_name = CString::new(name).map_err(|_error| GpuError::NoKernel(name.to_owned()))?;
        let mut function = std::ptr::null_mut();
        // SAFETY: the module is loaded and the name NUL-terminated.
        let result =
            unsafe { (api.cu_module_get_function)(&mut function, self.module, c_name.as_ptr()) };
        if result != CUDA_SUCCESS {
            return Err(GpuError::NoKernel(name.to_owned()));
        }
        Ok(Kernel {
            _module: self,
            function,
        })
    }
}

impl Drop for Module {
    fn drop(&mut self) {
        if let Ok(api) = self.gpu.current() {
            // SAFETY: unloaded once, by its owner.
            unsafe {
                (api.cu_module_unload)(self.module);
            }
        }
    }
}

/// A kernel in a loaded [`Module`].
pub struct Kernel<'m> {
    _module: &'m Module,
    function: CuFunction,
}

/// Device memory, freed on drop.
pub struct DeviceBuffer {
    gpu: Gpu,
    address: CuDevicePtr,
    bytes: usize,
}

impl DeviceBuffer {
    /// The device address, to pass to a kernel.
    pub fn arg(&self) -> KernelArg {
        KernelArg::Buffer(self.address)
    }

    pub fn len_bytes(&self) -> usize {
        self.bytes
    }

    /// Copies `data` to the start of the buffer.
    pub fn copy_from<T: Copy>(&self, data: &[T]) -> Result<(), GpuError> {
        let bytes = std::mem::size_of_val(data);
        if bytes > self.bytes {
            return Err(GpuError::OutOfBounds {
                requested: bytes,
                available: self.bytes,
            });
        }
        let api = self.gpu.current()?;
        // SAFETY: `bytes` is within both the host slice and the buffer.
        api.check("cuMemcpyHtoD", unsafe {
            (api.cu_memcpy_htod)(self.address, data.as_ptr().cast(), bytes)
        })
    }

    /// Copies the start of the buffer into `out`.
    pub fn copy_to<T: Copy>(&self, out: &mut [T]) -> Result<(), GpuError> {
        let bytes = std::mem::size_of_val(out);
        if bytes > self.bytes {
            return Err(GpuError::OutOfBounds {
                requested: bytes,
                available: self.bytes,
            });
        }
        let api = self.gpu.current()?;
        // SAFETY: `bytes` is within both the host slice and the buffer.
        api.check("cuMemcpyDtoH", unsafe {
            (api.cu_memcpy_dtoh)(out.as_mut_ptr().cast(), self.address, bytes)
        })
    }
}

impl Drop for DeviceBuffer {
    fn drop(&mut self) {
        if let Ok(api) = self.gpu.current() {
            // SAFETY: freed once, by its owner.
            unsafe {
                (api.cu_mem_free)(self.address);
            }
        }
    }
}
