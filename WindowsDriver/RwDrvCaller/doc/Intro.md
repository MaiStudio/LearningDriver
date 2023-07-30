# RwDrvCaller

## 1. Introduction

This project is a sample of calling the RwDrv.sys from [RW-Everything](http://rweverything.com/), a powerful utility for hardware related engineers.

## 2. Development Environment

Visual Studio 2008  with Windows x64

## 3. Reverse Engineering

According to the analysis of ESET-LoJax.pdf, RW tool embeds a driver called RwDrv.sys, it can use to read/write on almost all the low-level settings, and it supports the IOCTLs Code as below:

| IOCTL Code | Description                                         |
| ---------- | --------------------------------------------------- |
| 0x22280c   | Writes to memory mapped I/O space                   |
| 0x222808   | Reads from memory mapped I/O space                  |
| 0x222840   | Reads a dword from given PCI Configuration Register |
| 0x222834   | Writes a byte to given PCI Configuration Register   |
| 0x222848   | Read MSR                                            |
| 0x22284C   | Write MSR                                           |

therefore,  we can pass through the DeviceIoControl function to do some hardware modification.

