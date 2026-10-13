//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 13, 2026
//!
//! Functionality:
//! - A faithful copy of the allocation and collection paths of the mark-sweep
//!   collector that preceded the size-class heap (every object a `std::alloc` +
//!   `Box::pin(ObjectInfo)` + insert into an `RwLock<HashMap>`; tracing by
//!   cloning `Type`s per field), kept only so the benchmark can compare against
//!   it. Migration is not reproduced.
use std::{
    alloc::Layout,
    borrow::Cow,
    collections::{HashMap, VecDeque},
    pin::Pin,
    ptr::NonNull,
};

use codira_memory::{
    gc::{GcPtr, RawGcPtr},
    Type, TypeKind,
};
use parking_lot::RwLock;

#[repr(C)]
pub struct ArrayHeader {
    pub length: usize,
    pub capacity: usize,
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum Color {
    White,
    Gray,
    Black,
}

#[repr(C)]
union ObjectInfoData {
    ptr: NonNull<u8>,
    array: NonNull<ArrayHeader>,
}

#[repr(C)]
struct ObjectInfo {
    data: ObjectInfoData,
    roots: u32,
    color: Color,
    ty: Type,
}

unsafe impl Send for ObjectInfo {}
unsafe impl Sync for ObjectInfo {}

impl ObjectInfo {
    fn layout(&self) -> Layout {
        match self.ty.kind() {
            TypeKind::Array(array) => {
                let capacity = unsafe { self.data.array.as_ref().capacity };
                array_layout(&array.element_type(), capacity)
            }
            _ => self.ty.value_layout(),
        }
    }
}

fn array_layout(element: &Type, length: usize) -> Layout {
    let element_layout = element.reference_layout();
    let elements = Layout::from_size_align(
        element_layout.pad_to_align().size() * length,
        element_layout.align(),
    )
    .unwrap();
    Layout::new::<ArrayHeader>().extend(elements).unwrap().0
}

#[derive(Default)]
pub struct Stats {
    pub allocated_memory: usize,
}

/// The previous collector.
#[derive(Default)]
pub struct LegacyMarkSweep {
    objects: RwLock<HashMap<GcPtr, Pin<Box<ObjectInfo>>>>,
    stats: RwLock<Stats>,
}

impl LegacyMarkSweep {
    pub fn alloc(&self, ty: &Type) -> GcPtr {
        assert!(ty.is_concrete());
        let ptr = NonNull::new(unsafe { std::alloc::alloc_zeroed(ty.value_layout()) })
            .expect("failed to allocate memory for new object");
        let object = Box::pin(ObjectInfo {
            data: ObjectInfoData { ptr },
            ty: ty.clone(),
            roots: 0,
            color: Color::White,
        });
        let size = object.layout().size();
        let handle: GcPtr = (&*object.as_ref() as *const _ as RawGcPtr).into();
        self.objects.write().insert(handle, object);
        self.stats.write().allocated_memory += size;
        handle
    }

    pub fn alloc_array(&self, ty: &Type, n: usize) -> GcPtr {
        let element = ty.as_array().unwrap().element_type();
        let layout = array_layout(&element, n);
        let mut header: NonNull<ArrayHeader> =
            NonNull::new(unsafe { std::alloc::alloc_zeroed(layout).cast() }).unwrap();
        unsafe {
            header.as_mut().length = n;
            header.as_mut().capacity = n;
        }
        let object = Box::pin(ObjectInfo {
            data: ObjectInfoData { array: header },
            ty: ty.clone(),
            roots: 0,
            color: Color::White,
        });
        let size = object.layout().size();
        let handle: GcPtr = (&*object.as_ref() as *const _ as RawGcPtr).into();
        self.objects.write().insert(handle, object);
        self.stats.write().allocated_memory += size;
        handle
    }

    pub fn root(&self, handle: GcPtr) {
        let _lock = self.objects.write();
        let info = RawGcPtr::from(handle) as *mut ObjectInfo;
        unsafe { (*info).roots += 1 };
    }

    pub fn collect(&self) -> bool {
        let mut objects = self.objects.write();

        let mut roots = objects
            .values()
            .filter_map(|obj| {
                if obj.roots > 0 {
                    Some(obj.as_ref().get_ref() as *const _ as *mut ObjectInfo)
                } else {
                    None
                }
            })
            .collect::<VecDeque<_>>();

        while let Some(next) = roots.pop_front() {
            for reference in Trace::new(unsafe { NonNull::new_unchecked(next) }) {
                let handle: GcPtr = (reference.as_ptr() as RawGcPtr).into();
                let ref_ptr = objects.get_mut(&handle).expect("found invalid reference");
                if ref_ptr.color == Color::White {
                    let ptr = ref_ptr.as_ref().get_ref() as *const _ as *mut ObjectInfo;
                    unsafe { (*ptr).color = Color::Gray };
                    roots.push_back(ptr);
                }
            }
            unsafe { (*next).color = Color::Black };
        }

        let size_before = objects.len();
        objects.retain(|_, obj| {
            if obj.color == Color::Black {
                unsafe { obj.as_mut().get_unchecked_mut().color = Color::White };
                true
            } else {
                let layout = obj.layout();
                unsafe { std::alloc::dealloc(obj.data.ptr.as_ptr(), layout) };
                self.stats.write().allocated_memory -= layout.size();
                false
            }
        });
        size_before != objects.len()
    }
}

impl Drop for LegacyMarkSweep {
    fn drop(&mut self) {
        for (_, obj) in self.objects.get_mut().drain() {
            let layout = obj.layout();
            unsafe { std::alloc::dealloc(obj.data.ptr.as_ptr(), layout) };
        }
    }
}

// --- Tracing, as in the previous collector -------------------------------

struct Trace {
    stack: VecDeque<CompositeTrace>,
}

impl Trace {
    fn new(obj: NonNull<ObjectInfo>) -> Trace {
        let mut trace = Trace {
            stack: VecDeque::default(),
        };
        let obj_ref = unsafe { obj.as_ref() };
        match obj_ref.ty.kind() {
            TypeKind::Primitive(_) | TypeKind::Pointer(_) => {}
            TypeKind::Struct(_) => {
                trace.stack.push_back(CompositeTrace::Struct(StructTrace {
                    struct_ptr: unsafe { obj_ref.data.ptr },
                    struct_type: obj_ref.ty.clone(),
                    field_index: 0,
                }));
            }
            TypeKind::Array(arr) => {
                let header = unsafe { obj_ref.data.array };
                let element_ty = arr.element_type();
                let element_layout = element_ty.reference_layout();
                let (_, offset) = Layout::new::<ArrayHeader>().extend(element_layout).unwrap();
                trace.stack.push_back(CompositeTrace::Array(ArrayTrace {
                    next: unsafe {
                        NonNull::new_unchecked(header.as_ptr().cast::<u8>().add(offset))
                    },
                    remaining: unsafe { header.as_ref().length },
                    stride: element_layout.pad_to_align().size(),
                    element_ty,
                }));
            }
        }
        trace
    }
}

impl Iterator for Trace {
    type Item = NonNull<ObjectInfo>;

    fn next(&mut self) -> Option<Self::Item> {
        loop {
            let top = self.stack.back_mut()?;
            let event = match top {
                CompositeTrace::Struct(s) => s.next(),
                CompositeTrace::Array(a) => a.next(),
            };
            match event {
                None => {
                    self.stack.pop_back();
                }
                Some(TraceEvent::Reference(r)) => return Some(r),
                Some(TraceEvent::InlineStruct(s)) => {
                    self.stack.push_back(CompositeTrace::Struct(s));
                }
            }
        }
    }
}

enum CompositeTrace {
    Struct(StructTrace),
    Array(ArrayTrace),
}

enum TraceEvent {
    Reference(NonNull<ObjectInfo>),
    InlineStruct(StructTrace),
}

impl TraceEvent {
    fn new(ptr: NonNull<u8>, ty: Cow<'_, Type>) -> Option<TraceEvent> {
        match ty.kind() {
            TypeKind::Primitive(_) | TypeKind::Pointer(_) => None,
            TypeKind::Struct(s) => {
                if s.is_gc_struct() {
                    let deref = unsafe { ptr.cast::<NonNull<ObjectInfo>>().as_ref() };
                    Some(TraceEvent::Reference(*deref))
                } else {
                    Some(TraceEvent::InlineStruct(StructTrace {
                        struct_ptr: ptr,
                        struct_type: ty.into_owned(),
                        field_index: 0,
                    }))
                }
            }
            TypeKind::Array(_) => Some(TraceEvent::Reference(ptr.cast())),
        }
    }
}

struct StructTrace {
    struct_ptr: NonNull<u8>,
    struct_type: Type,
    field_index: usize,
}

impl Iterator for StructTrace {
    type Item = TraceEvent;

    fn next(&mut self) -> Option<Self::Item> {
        let struct_ty = self.struct_type.as_struct()?;
        let fields = struct_ty.fields();
        while self.field_index < fields.len() {
            let field = fields.get(self.field_index).unwrap();
            self.field_index += 1;
            let field_ptr =
                unsafe { NonNull::new_unchecked(self.struct_ptr.as_ptr().add(field.offset())) };
            if let Some(event) = TraceEvent::new(field_ptr, Cow::Owned(field.ty())) {
                return Some(event);
            }
        }
        None
    }
}

struct ArrayTrace {
    next: NonNull<u8>,
    remaining: usize,
    stride: usize,
    element_ty: Type,
}

impl Iterator for ArrayTrace {
    type Item = TraceEvent;

    fn next(&mut self) -> Option<Self::Item> {
        while self.remaining > 0 {
            let element = self.next;
            self.remaining -= 1;
            self.next = unsafe { NonNull::new_unchecked(self.next.as_ptr().add(self.stride)) };
            if let Some(event) = TraceEvent::new(element, Cow::Borrowed(&self.element_ty)) {
                return Some(event);
            }
        }
        None
    }
}
