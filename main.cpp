#include "stdafx.h"
#include "resource.h"
#include "../SDK/foobar2000.h"
#include "../../pfc/pfc.h"

//Global GUIDs...should be moved to guid.h
static GUID guid1 = { 0x2bffadc1, 0x63e6, 0x4119, { 0xa8, 0x56, 0xba, 0x79, 0xa6, 0x6e, 0x1e, 0xed } };
static GUID guid2 = { 0x484e0ed3, 0xfeaf, 0x4c5e, { 0xa1, 0x44, 0x94, 0xaf, 0x16, 0x76, 0x81, 0xf3 } };
static GUID guid3 = { 0xb0584229, 0x26f8, 0x4d7f, { 0x83, 0x4e, 0x74, 0xd4, 0x03, 0xf8, 0xb0, 0xd6 } };
static GUID guid4 = { 0xd3b7d6f9, 0x88ef, 0x4f54, { 0x96, 0xf0, 0x41, 0x76, 0xa1, 0x78, 0xd5, 0x2a } };
static GUID guid5 = { 0x38460f7b, 0x21aa, 0x47ab, { 0xbd, 0x0, 0x3b, 0xd0, 0xc8, 0xdb, 0x22, 0x73 } };
static GUID guid6 = { 0xeb8001dd, 0xecf2, 0x4b41, { 0xa9, 0x57, 0x26, 0x56, 0xea, 0x2d, 0x4b, 0xdc } };
static GUID guid7 = { 0x27b7307, 0x56cf, 0x465c, { 0x9f, 0x96, 0x25, 0x5, 0x9a, 0xd, 0x5d, 0x98 } };
static GUID guid8 = { 0xa9a59387, 0x91cf, 0x49d5, { 0x91, 0x94, 0xcc, 0xcd, 0x85, 0x79, 0xc1, 0x88 } };
static GUID guid9 = { 0x691ca285, 0x59b1, 0x4779, { 0x89, 0xda, 0xbf, 0xa, 0x65, 0x74, 0x84, 0x75 } };

// Declaration of your component's version information
// Since foobar2000 v1.0 having at least one of these in your DLL is mandatory to let the troubleshooter tell different versions of your component apart.
// Note that it is possible to declare multiple components within one DLL, but it's strongly recommended to keep only one declaration per DLL.
// As for 1.1, the version numbers are used by the component update finder to find updates; for that to work, you must have ONLY ONE declaration per DLL. If there are multiple declarations, the component is assumed to be outdated and a version number of "0" is assumed, to overwrite the component with whatever is currently on the site assuming that it comes with proper version numbers.
DECLARE_COMPONENT_VERSION("Foo Queue", "0.9.0", "Foobar component to FooQueue Android app");
// This will prevent users from renaming your component around (important for proper troubleshooter behaviors) or loading multiple instances of it.
VALIDATE_COMPONENT_FILENAME("foo_queue.dll");
