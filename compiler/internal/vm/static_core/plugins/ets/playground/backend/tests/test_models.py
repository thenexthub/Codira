#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import pytest
from pydantic import ValidationError

from src.arkts_playground.models.common import VerificationMode
from src.arkts_playground.models.compile import CompileRequestModel


class TestVerificationMode:
    """Tests for VerificationMode enum."""

    @pytest.mark.parametrize("mode", list(VerificationMode))
    def test_valid_verification_modes(self, mode: VerificationMode):
        """Test that all valid verification modes are accepted."""
        model = CompileRequestModel(code="let x = 1", verification_mode=mode)
        assert model.verification_mode == mode

    def test_invalid_verification_mode(self):
        """Test that invalid verification modes are rejected."""
        with pytest.raises(ValidationError) as exc_info:
            CompileRequestModel(code="let x = 1", verification_mode="invalid-mode")
        assert "verification_mode" in str(exc_info.value)

    def test_default_verification_mode(self):
        """Test that default verification mode is 'ahead-of-time'."""
        model = CompileRequestModel(code="let x = 1")
        assert model.verification_mode == VerificationMode.AHEAD_OF_TIME


class TestCompileRequestModel:
    """Tests for CompileRequestModel."""

    def test_minimal_request(self):
        """Test creating model with only required field."""
        model = CompileRequestModel(code="let x = 1")
        assert model.code == "let x = 1"
        assert model.options == {}
        assert model.disassemble is False
        assert model.verifier is False
        assert model.verification_mode == VerificationMode.AHEAD_OF_TIME

    def test_full_request(self):
        """Test creating model with all fields."""
        model = CompileRequestModel(
            code="let x = 1",
            options={"--opt-level": "2"},
            disassemble=True,
            verifier=True,
            verification_mode=VerificationMode.ON_THE_FLY,
        )
        assert model.code == "let x = 1"
        assert model.options == {"--opt-level": "2"}
        assert model.disassemble is True
        assert model.verifier is True
        assert model.verification_mode == VerificationMode.ON_THE_FLY

    def test_verification_mode_disabled(self):
        """Test model with disabled verification mode."""
        model = CompileRequestModel(code="let x = 1", verification_mode=VerificationMode.DISABLED)
        assert model.verification_mode == VerificationMode.DISABLED

    def test_empty_code(self):
        """Test that empty code is allowed."""
        model = CompileRequestModel(code="")
        assert model.code == ""

    def test_missing_code_raises_error(self):
        """Test that missing code field raises validation error."""
        with pytest.raises(ValidationError) as exc_info:
            CompileRequestModel()
        assert "code" in str(exc_info.value)
