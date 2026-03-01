#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

# frozen_string_literal: true

#
# Huawei Technologies Co.,Ltd.
require 'optparse'
require 'yaml'
require 'erb'

class ReportMd
  attr_accessor :rep

  def initialize(rep)
    @template_file = File.join(__dir__, 'templates', 'report.erb')
    @rep = rep
  end

  def generate
    $stdout.write(render)
  end

  private

  def render
    @template = File.read(@template_file)
    ERB.new(@template, trim_mode: '%-').result(binding)
  end
end
