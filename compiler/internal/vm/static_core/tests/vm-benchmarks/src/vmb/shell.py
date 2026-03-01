#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import re
import os
import logging
import signal
import time
from typing import Union, Optional
from pathlib import Path
from subprocess import Popen, PIPE, CalledProcessError, check_output
from threading import Thread, Timer
from dataclasses import dataclass
from tempfile import mktemp
from vmb.helpers import Singleton

log = logging.getLogger('vmb')
tm_re = re.compile(
    r"(?:Elapsed.*\(h:mm:ss or m:ss\)|Real time)"
    r"[^:]*:\s*(?:(\d*):)?(\d*)(?:.(\d*))?")
rss_re = re.compile(r"(?:Maximum resident set size|Max RSS)[^:]*:\s*(\d*)")


@dataclass
class ShellResult:

    # Default initial result is 'failure'
    ret: int = -13
    out: str = ''
    err: str = ''
    tm: float = 0.0
    rss: int = 0

    def grep(self, regex: str) -> str:
        out = self.out.split("\n")
        err = self.err.split("\n")
        for line in out + err:
            m = re.search(regex, line)
            if m:
                if len(m.groups()) < 1:
                    return m.group()
                return m.group(1)
        return ''

    def replace_out(self, regex: re.Pattern, repl: str = '') -> None:
        out = self.out.split("\n")
        new_lines = [regex.sub(repl, line) for line in out if line.strip()]
        self.out = "\n".join(new_lines)

    def set_ret_val(self) -> None:
        if not self.out:
            log.error("No shell output")
            self.ret = -13
        matches = re.search(r"__RET_VAL__=(\d*)", self.out)
        if not matches:
            log.error("No shell ret val; out:")
            self.ret = -13
        else:
            try:
                self.ret = int(matches.groups()[0])
            except ValueError:
                log.error('Error parsing return code')
                self.ret = -14

    def set_time(self) -> None:
        # expecting output of '\time -v' to stderr
        if not self.err:
            return
        tm_val = re.search(tm_re, self.err)
        if tm_val:
            tmp = tm_val.groups()
            if tmp[0] is None:
                self.tm = float(str(tmp[1]) + "." + tmp[2])
            else:
                self.tm = int(tmp[0]) * 60 + float(str(tmp[1]) + "." + tmp[2])
            self.tm = round(self.tm, 5)
        else:
            self.tm = 0.0
        self.tm *= 1e9
        rss_val = re.search(rss_re, self.err)
        if rss_val:
            self.rss = int(rss_val.group(1))
        else:
            self.rss = 0

    def log_output(self) -> None:
        if self.ret != 0:
            if self.out:
                log.error(self.out)
            err = self.err.split("\n")[:3] if self.err else []
            for line in err:
                log.error(line.strip())
        else:
            if self.out:
                log.debug(self.out)


class ShellBase(metaclass=Singleton):

    def __init__(self, timeout: Optional[float] = None, no_run: bool = False) -> None:
        self._timeout = timeout
        self.no_run = no_run
        self.taskset = ''

    @staticmethod
    def timed_cmd(cmd: str) -> str:
        return f"\\time -v env {cmd}"

    def effective_timeout(self, timeout: Optional[float] = None) -> Optional[float]:
        # Note: self._timeout=None so default behaivior is to wait forever
        to = timeout if timeout else self._timeout
        if timeout is not None and self._timeout is not None:
            to = max(timeout, self._timeout)
        return to

    def run(self,
            cmd: str,
            measure_time: bool = False,
            timeout: Optional[float] = None,
            cwd: str = '') -> ShellResult:
        raise NotImplementedError

    def run_async(self, cmd: str) -> None:
        raise NotImplementedError

    def run_syslog(self, cmd: str,
                   finished_marker: str,
                   measure_time: bool = False,
                   timeout: Optional[float] = None,
                   cwd: str = '',
                   ping_interval: int = 5,
                   tag: str = 'VMB') -> ShellResult:
        raise NotImplementedError

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def install(self, package: Union[str, Path], name: str = '') -> ShellResult:
        raise NotImplementedError

    def get_filesize(self, filepath: Union[str, Path]) -> int:
        if os.path.exists(str(filepath)):
            return os.stat(str(filepath)).st_size
        return 0

    def grep_output(self, cmd: str, regex: str) -> str:
        r = self.run(cmd=cmd)
        return r.grep(regex)

    def set_affinity(self, arg: str) -> None:
        """Set affinity mask for processes.

        Effective only on devices, so hardcoding path
        """
        self.taskset = f'/system/bin/taskset -a {arg}'


class ShellHost(ShellBase):

    def __init__(self, timeout: Optional[float] = None, no_run: bool = False) -> None:
        super().__init__(timeout=timeout, no_run=no_run)
        self.is_win = 'nt' == os.name

    def run(self,
            cmd: str,
            measure_time: bool = False,
            timeout: Optional[float] = None,
            cwd: str = '') -> ShellResult:
        if self.no_run:
            log.info('\n%s\n', cmd)
            return ShellResult(ret=0, out='OK\n__RET_VAL__=0\n')
        return self.__run(
            cmd, measure_time=measure_time, timeout=timeout, cwd=cwd)

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def run_syslog(self, cmd: str,
                   finished_marker: str,
                   measure_time: bool = False,
                   timeout: Optional[float] = None,
                   cwd: str = '',
                   ping_interval: int = 5,
                   tag: str = 'VMB') -> ShellResult:
        raise NotImplementedError

    def run_async(self, cmd: str) -> None:
        if self.no_run:
            log.info('\n%s\n', cmd)
            return

        def run_shell():
            # pylint: disable-next=all
            return Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)  # NOQA

        log.debug('Async cmd: %s', cmd)
        async_trhead = Thread(target=run_shell)
        async_trhead.daemon = True
        async_trhead.start()

    def __run(self,
              cmd: str,
              measure_time: bool = False,
              timeout: Optional[float] = None,
              cwd: str = '') -> ShellResult:
        if self.no_run:
            log.info('\n%s\n', cmd)
            return ShellResult(ret=0, out='OK\n__RET_VAL__=0\n')
        if measure_time and not self.is_win:
            cmd = self.timed_cmd(cmd)
        exec_fn = self.__exec_process_win if self.is_win else self.__exec_process
        result = exec_fn(cmd, cwd=cwd, timeout=self.effective_timeout(timeout))
        if measure_time:
            result.set_time()
        result.log_output()
        return result

    def __exec_process(self, cmd: str, cwd: str = '',
                       timeout: Optional[float] = None) -> ShellResult:
        result = ShellResult()
        log.debug(cmd)
        log.trace('CWD="%s" Timeout=[%s]', cwd, timeout)
        # pylint: disable-next=all
        with Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE,  # NOQA
                   cwd=(cwd if cwd else None),
                   preexec_fn=os.setsid) as proc:
            if timeout is not None:
                timer = Timer(timeout,
                              lambda x: os.killpg(
                                  os.getpgid(x.pid), signal.SIGKILL), [proc])
                timer.start()
            out, err = proc.communicate(timeout=timeout)
            if timeout is not None:
                timer.cancel()
            ret_code = proc.poll()
            if ret_code is not None:
                result.ret = ret_code
            result.out = str(out.decode('utf-8', errors='replace'))
            result.err = str(err.decode('utf-8', errors='replace'))
        return result

    def __exec_process_win(self, cmd: str, cwd: str = '',
                           timeout: Optional[float] = None) -> ShellResult:
        """Win realisation for exec process.

        In host: No shell; No RSS; No time; No kill group;
        On device: using 'time' shell builtin and cmd stderr redirection
        """
        result = ShellResult()
        log.debug(cmd)
        try:
            out = check_output(cmd, shell=False, cwd=(cwd if cwd else None),
                               timeout=timeout)
            result.ret = 0
            result.out = str(out.decode('utf-8', errors='replace'))
        except CalledProcessError as e:
            result.ret = e.returncode
            result.out = str(e.output)
        return result


class ShellDevice(ShellBase):
    def __init__(self,
                 dev_sh: str,
                 timeout: Optional[float] = None,
                 tmp_dir: str = '/data/local/tmp/vmb',
                 no_run: bool = False) -> None:
        super().__init__(timeout=timeout, no_run=no_run)
        self._sh = ShellHost(no_run=no_run)
        self._devsh = dev_sh
        self.tmp_dir = tmp_dir
        self.stderr_out = tmp_dir + '/vmb-stderr.out'

    def run(self, cmd: str,
            measure_time: bool = False,
            timeout: Optional[float] = None,
            cwd: str = '') -> ShellResult:
        if self.no_run:
            log.info('\n%s\n', cmd)
            return ShellResult(ret=0, out='OK\n__RET_VAL__=0\n')
        redir = ''
        if measure_time:
            cmd = f"\\time -v {self.taskset} env {cmd}"
            redir = f' 2>{self.stderr_out}'
        cwd = f'cd {Path(cwd).as_posix()}; ' if cwd else ''
        # Single/Double quote problem for cross-platform shell:
        # 1) there is no proper way to have '>' inside single quotes on Windows
        # 2) on the other hand "echo $?" will be prematurely expanded on Unix host
        q = '"' if self._sh.is_win else "'"
        res = self._sh.run(
            f'{self._devsh} shell {q}{cwd}({cmd}){redir}; echo __RET_VAL__=$?{q}',
            timeout=timeout,
            measure_time=False)
        res.set_ret_val()
        if measure_time:
            stderr_host = mktemp(prefix='vmb-')
            self.pull(self.stderr_out, stderr_host)
            self._sh.run(f'{self._devsh} shell {q}rm -f {self.stderr_out}{q}')
            if not Path(stderr_host).exists():
                res.err = 'Pull from device failed'
                return res
            with open(stderr_host, 'r', encoding="utf-8") as f:
                res.err = f.read()
            os.remove(stderr_host)
            res.set_time()
        else:
            res.err = ''
        return res

    def run_syslog(self, cmd: str,
                   finished_marker: str,
                   measure_time: bool = False,
                   timeout: Optional[float] = None,
                   cwd: str = '',
                   ping_interval: int = 5,
                   tag: str = 'VMB') -> ShellResult:
        raise NotImplementedError

    def run_async(self, cmd: str) -> None:
        self._sh.run_async(f"{self._devsh} shell '{cmd}'")

    def get_filesize(self, filepath: Union[str, Path]) -> int:
        filepath = Path(filepath).as_posix()
        res = self.run(f"stat -c '%s' {filepath}")
        size = 0
        if res.ret == 0 and res.out:
            try:
                size = int(res.out.split("\n")[0])
            except Exception:  # pylint: disable=broad-exception-caught
                log.warning('Error getting size of "%s"', filepath)
        return size

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        raise NotImplementedError

    def install(self, package: Union[str, Path], name: str = '') -> ShellResult:
        raise NotImplementedError

    def mk_tmp_dir(self):
        res = self.run(f'mkdir -p {self.tmp_dir}')
        if res.ret != 0:
            raise RuntimeError('Device connection failed!\n'
                               f'{res.out}\n{res.err}')


class ShellAdb(ShellDevice):
    binname = f"a{'d'}b"

    def __init__(self,
                 dev_serial: str = '',
                 timeout: Optional[float] = None,
                 tmp_dir: str = '/data/local/tmp/vmb',
                 no_run: bool = False) -> None:
        super().__init__(
            f"{os.environ.get(self.binname.upper(), self.binname)}",
            timeout=timeout,
            tmp_dir=tmp_dir,
            no_run=no_run)
        if dev_serial:
            self._devsh = f'{self._devsh} -s {dev_serial}'
        self.mk_tmp_dir()

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        posix_dst = Path(dst).as_posix()
        return self._sh.run(f'{self._devsh} push {src} {posix_dst}',
                            measure_time=False)

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        return self._sh.run(f'{self._devsh} pull {src} {dst}',
                            measure_time=False)

    def install(self, package: Union[str, Path], name: str = '') -> ShellResult:
        raise NotImplementedError


class ShellHdc(ShellDevice):
    # hardcoded tag and app name for now
    hilog_re = re.compile(r'^.*com.example.helllopanda/VMB: ')

    def __init__(self,
                 dev_serial: str = '',
                 dev_host: str = '',
                 timeout: Optional[float] = None,
                 tmp_dir: str = '/data/local/tmp/vmb',
                 no_run: bool = False) -> None:
        # -l0 because of HDC mutex file permission messages
        # -p (undocumented) due to poor hdc performance
        super().__init__(f"{os.environ.get('HDC', 'hdc')} -p -l0",
                         timeout=timeout,
                         tmp_dir=tmp_dir,
                         no_run=no_run)
        if dev_serial:
            self._devsh = f'{self._devsh} -t {dev_serial}'
        if dev_host:
            self._devsh = f'{self._devsh} -s {dev_host}'
        self.mk_tmp_dir()

    def push(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        posix_dst = Path(dst).as_posix()
        return self._sh.run(f'{self._devsh} file send {src} "{posix_dst}"',
                            measure_time=False)

    def pull(self,
             src: Union[str, Path],
             dst: Union[str, Path]) -> ShellResult:
        posix_src = Path(src).as_posix()
        return self._sh.run(f'{self._devsh} file recv {posix_src} {dst}',
                            measure_time=False)

    def install(self, package: Union[str, Path], name: str = '') -> ShellResult:
        if name:
            self._sh.run(f'{self._devsh} uninstall {name}', measure_time=False)
        return self._sh.run(f'{self._devsh} aa install {package}', measure_time=False)

    def grab_log(self, tag: str, finished_marker: str) -> Optional[ShellResult]:
        opts = f' -T {tag}' if tag else ''
        res = self.run(f'hilog -x{opts}')
        if res.grep(finished_marker):
            # success. strip hilog data
            res.replace_out(self.hilog_re)
            return res
        return None

    def run_syslog(self, cmd: str,
                   finished_marker: str,
                   measure_time: bool = False,
                   timeout: Optional[float] = None,
                   cwd: str = '',
                   ping_interval: int = 5,
                   tag: str = 'VMB') -> ShellResult:
        self.run('rm -f /data/log/faultlog/faultlogger/*')
        self.run('hilog -r')  # clear log buffer
        res = self.run(cmd=cmd, measure_time=measure_time, cwd=cwd)
        if res.ret != 0:
            log.error('Command failed. Skippping results.')
            return res
        res_log = None
        if 0 == ping_interval:  # synchronous cmd
            res_log = self.grab_log(tag, finished_marker)
        else:  # async cmd
            to = 30 if timeout is None else timeout
            elapsed = 0
            while elapsed < to:
                log.debug("Waiting  %d sec for [%s]", ping_interval, finished_marker)
                time.sleep(ping_interval)
                elapsed += ping_interval
                res_log = self.grab_log(tag, finished_marker)
                if res_log:
                    break
        if res_log:
            res.out = res_log.out
            return res
        # error. save full log
        res.ret = 1
        try:
            res.out = self.run('cat /data/log/faultlog/faultlogger/* | head -40').out
        except Exception:  # pylint: disable=broad-exception-caught
            log.warning('Error getting fault logs!')
        return res
