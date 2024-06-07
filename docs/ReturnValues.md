# Examples of Return Values

## envuGetOS* functions

Examples of return values for envuGetOS(), envuGetOSVersion(), and envuGetOSProductName().

| OS | Version | Product Name |
| -- | -- | -- |
| Darwin | 19.5.0 | Mac OS X 10.15.5 |
| Windows | 10.0.19045 | Microsoft Windows 10 Home |
| Linux | 5.15.0 | Ubuntu 20.04.6 LTS |
| Linux | 6.1.92 | Alpine Linux v3.18 |
| FreeBSD | 14.0 | FreeBSD 14.0 |
| NetBSD | 9.3 | NetBSD 9.3 |
| OpenBSD | 7.4 | OpenBSD 7.4 |
| SunOS | 5.11 | OpenIndiana Hipster 2024.04 |
| Haiku | 1 | Haiku R1/beta4 |

## envuGetFullPath

Examples of input paths and return values for envuGetFullPath()

### Windows

| Input | Return |
| -- | -- |
| "C:\usr\lib\." | "C:\usr\lib" |
| "C:\usr\lib\.." | "C:\usr" |
| "C:\usr\.\lib" | "C:\usr\lib" |
| "C:\usr\..\lib" | "C:\lib" |
| "C:\." | "C:\" |
| "C:\.." | "C:\" |
| "C:\usr\lib\" | "C:\usr\lib" |
| "usr" | "C:\path\to\cwd\usr" |
| "." | "C:\path\to\cwd" |
| "" | "C:\path\to\cwd" |
| NULL | NULL |

### Linux/Unix

| Input | Return |
| -- | -- |
| "/usr/lib" | "/usr/lib" |
| "/usr/lib" | "/usr" |
| "/usr/./lib" | "/usr/lib" |
| "/usr/../lib" | "/lib" |
| "/." | "/" |
| "/.." | "/" |
| "/usr/lib/" | "/usr/lib" |
| "usr" | "/path/to/cwd/usr" |
| "." | "/path/to/cwd" |
| "" | "/path/to/cwd" |
| NULL | NULL |

## envuGetDirectory

Examples of input paths and return values for envuGetDirecotry()

| Input | Return |
| -- | -- |
| "/usr/lib" | "/usr" |
| "/usr/" | "/" |
| "usr" | "." |
| "usr/" | "." |
| "/" | "/" |
| "." | "." |
| ".." | "." |
| "" | "." |
| NULL | NULL |
