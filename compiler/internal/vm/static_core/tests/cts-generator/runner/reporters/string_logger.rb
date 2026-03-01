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

module TestRunner
  module Reporters
    # StringLogger is intended to keep all single test output.
    class StringLogger
      def initialize
        @content = StringIO.new
      end

      def log(level, *args)
        if @content.closed_write?
          raise IOError,
                "#{self.class} is closed for writing. It is possible that epilogue() is called."
        end

        @content.puts(args) if level <= $VERBOSITY
      end

      def string
        @content.string
      end

      def close
        @content.close
      end
    end

    class SeparateFileLogger
      def initialize(log_file)
        @file = File.new(log_file, 'w')
      end

      def log(_level, *args)
        @file.write(*args, "\n")
      end

      def string
        ''
      end

      def close
        @file.close
      end
    end
  end
end
