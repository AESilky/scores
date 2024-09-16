# Licenses and Acknowledgements

This contains sections with an acknowledgement and license information for
the 3rd-Party libraries and code used within the Scores software.

## Raspberry Pi Pico SDK Examples

The Pico SDK is used to build the operating code. Various Pico SDK Examples
provided guidance for using the Pico and some of the operations. Code used for
the Scores operating software was rarely copied from any of the examples. If
any significant portion was copied it has been noted in the source file where
it was used. That being said, any code used from the SDK or the SDK Examples is:

* Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The SDK is available here: https://github.com/raspberrypi/pico-examples

## FatFs - Generic FAT Filesystem Module - ChaN

FatFs is used for the SD Card support. The SD Card is used to maintain
configuration and other files. The code is within the `src/lib` directory,
and the full license file is included there.

The following is for easy reference:

```
/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  Rx.xx                               /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 20xx, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/
```

## SD SPI Library - Carl John Kugler III

The SD SPI Library is used for the SD Card support. The SD Card is used to
maintain configuration and other files. The code is within the `src/lib`
directory. The source files contain the copyright and license information.
and the full license file is included there.

The following is for easy reference:

```
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at:
http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
```
