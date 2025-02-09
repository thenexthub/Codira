/*
 * Copyright (c) NeXTHub Corporation. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 */

package langserver

import (
	"context"
	"io"
	"io/fs"
	"log"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/goplus/gop/x/jsonrpc2/stdio"
)

func fatal(err error) {
	log.Fatalln(err)
}

// -----------------------------------------------------------------------------

// ServeAndDialConfig represents the configuration of ServeAndDial.
type ServeAndDialConfig struct {
	// OnError is to customize how to process errors (optional).
	// It should panic in any case.
	OnError func(err error)
}

const (
	logPrefix = "serve-"
	logSuffix = ".log"
)

func logFileOf(gopDir string, pid int) string {
	return gopDir + logPrefix + strconv.Itoa(pid) + logSuffix
}

func isLog(fname string) bool {
	return strings.HasSuffix(fname, logSuffix) && strings.HasPrefix(fname, logPrefix)
}

func pidByName(fname string) int {
	pidText := fname[len(logPrefix) : len(fname)-len(logSuffix)]
	if ret, err := strconv.ParseInt(pidText, 10, 0); err == nil {
		return int(ret)
	}
	return -1
}

func killByPid(pid int) {
	if pid < 0 {
		return
	}
	if proc, err := os.FindProcess(pid); err == nil {
		proc.Kill()
	}
}

func tooOld(d fs.DirEntry) bool {
	if fi, e := d.Info(); e == nil {
		lastHour := time.Now().Add(-time.Hour)
		return fi.ModTime().Before(lastHour)
	}
	return false
}

// ServeAndDial executes a command as a LangServer, makes a new connection to it
// and returns a client of the LangServer based on the connection.
func ServeAndDial(conf *ServeAndDialConfig, gopCmd string, args ...string) Client {
	if conf == nil {
		conf = new(ServeAndDialConfig)
	}
	onErr := conf.OnError
	if onErr == nil {
		onErr = fatal
	}

	home, err := os.UserHomeDir()
	if err != nil {
		onErr(err)
	}
	gopDir := home + "/.gop/"
	err = os.MkdirAll(gopDir, 0755)
	if err != nil {
		onErr(err)
	}

	// logFile is where the LangServer application log saves to.
	// default is ~/.gop/serve-{pid}.log
	logFile := logFileOf(gopDir, os.Getpid())

	// clean too old logfiles, and kill old LangServer processes
	go func() {
		if fis, e := os.ReadDir(gopDir); e == nil {
			for _, fi := range fis {
				if fi.IsDir() {
					continue
				}
				if fname := fi.Name(); isLog(fname) && tooOld(fi) {
					os.Remove(gopDir + fname)
					killByPid(pidByName(fname))
				}
			}
		}
	}()

	in, w := io.Pipe()
	r, out := io.Pipe()

	var cmd *exec.Cmd
	go func() {
		defer r.Close()
		defer w.Close()

		f, err := os.Create(logFile)
		if err != nil {
			onErr(err)
		}
		defer f.Close()

		cmd = exec.Command(gopCmd, args...)
		cmd.Stdin = r
		cmd.Stdout = w
		cmd.Stderr = f
		err = cmd.Start()
		if err != nil {
			onErr(err)
		}

		newLogFile := logFileOf(gopDir, cmd.Process.Pid)
		os.Rename(logFile, newLogFile)

		cmd.Wait()
	}()

	c, err := Open(context.Background(), stdio.Dialer(in, out), func() {
		if cmd == nil {
			return
		}
		if proc := cmd.Process; proc != nil {
			log.Println("==> ServeAndDial: kill", proc.Pid, gopCmd, args)
			proc.Kill()
		}
	})
	if err != nil {
		onErr(err)
	}
	return c
}

// -----------------------------------------------------------------------------
