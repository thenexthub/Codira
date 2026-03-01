//==-- SystemZSchedule.td - SystemZ Scheduling Definitions ----*- tblgen -*-==//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

// Scheduler resources

// These resources are used to express decoder grouping rules.  The number of
// decoder slots needed by an instructions is normally one, but there are
// exceptions.
def NormalGr    : SchedWrite;
def Cracked     : SchedWrite;
def GroupAlone  : SchedWrite;
def GroupAlone2 : SchedWrite;
def GroupAlone3 : SchedWrite;
def BeginGroup  : SchedWrite;
def EndGroup    : SchedWrite;

// A SchedWrite added to other SchedWrites to make LSU latency parameterizable.
def LSULatency : SchedWrite;

// Operand WriteLatencies.
foreach L = 1 - 30 in def "WLat"#L : SchedWrite;

foreach L = 1 - 16 in
  def "WLat"#L#"LSU" : WriteSequence<[!cast<SchedWrite>("WLat"#L),
                                      LSULatency]>;

// ReadAdvances, used for the register operand next to a memory operand,
// modelling that the register operand is needed later than the address
// operands.
def RegReadAdv : SchedRead;

foreach Num = ["", "2", "3", "4", "5", "6"] in {
  // Fixed-point units
  def "FXa"#Num : SchedWrite;
  def "FXb"#Num : SchedWrite;
  def "FXU"#Num : SchedWrite;
  // Load/store unit
  def "LSU"#Num : SchedWrite;
  // Vector sub units (z13 and later)
  def "VecBF"#Num : SchedWrite;
  def "VecDF"#Num : SchedWrite;
  def "VecDFX"#Num : SchedWrite;
  def "VecMul"#Num : SchedWrite;
  def "VecStr"#Num : SchedWrite;
  def "VecXsPm"#Num : SchedWrite;
  // Floating point unit (zEC12 and earlier)
  def "FPU"#Num : SchedWrite;
  def "DFU"#Num : SchedWrite;
}

def VecFPd   : SchedWrite; // Blocking BFP div/sqrt unit (30 cycles).
def VecFPd20 : SchedWrite; // Blocking BFP div/sqrt unit, 20 cycles.

def VBU : SchedWrite; // Virtual branching unit

def MCD : SchedWrite; // Millicode

include "SystemZScheduleZ17.td"
include "SystemZScheduleZ16.td"
include "SystemZScheduleZ15.td"
include "SystemZScheduleZ14.td"
include "SystemZScheduleZ13.td"
include "SystemZScheduleZEC12.td"
include "SystemZScheduleZ196.td"
