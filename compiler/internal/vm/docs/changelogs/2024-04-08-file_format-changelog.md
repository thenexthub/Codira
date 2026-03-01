# 2024-04-08-file_format-changelog

This document describes change log with the following modifications:

* Version
* Proto
* IndexHeader

File format with this changes is incompatible with version <=12.0.0.0 of Runtime.

## Version
1. We update the version to 12.0.1.0

## Proto
1. We delete ProtoItem's data from file format from version 12.0.1.0.
2. We invalid the [proto_idx] field by setting [0xffff] in Method from version 12.0.1.0.

## IndexHeader
1. We delete the corresponding [field_idx_off] offsets-array from file format from version 12.0.1.0.
2. We invalid the [field_idx_size] & [field_idx_off] by setting [0xffffffff] in IndexHeader from version 12.0.1.0.
3. We delete the corresponding [proto_idx_off] offsets-array from file format from version 12.0.1.0.
4. We invalid the [proto_idx_size] & [proto_idx_off] by setting [0xffffffff] in IndexHeader from version 12.0.1.0.
