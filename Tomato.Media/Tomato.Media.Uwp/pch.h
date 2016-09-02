#pragma once
#define NOMINMAX
#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mfidl.h>
#include <ppltasks.h>
#include <algorithm>
#include <Mferror.h>
#include <mfapi.h>
#include <pplawait.h>