/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#pragma once

#include "hart/config.h"

#include "hart/base/atomic.h"
#include "hart/base/crt.h"
#include "hart/base/debug.h"
#include "hart/base/filesystem.h"
#include "hart/base/freelist.h"
#include "hart/base/matrix.h"
#include "hart/base/std.h"
#include "hart/base/threadlocalstorage.h"
#include "hart/base/time.h"
#include "hart/base/util.h"
#include "hart/base/uuid.h"
#include "hart/base/vec.h"

#include "hart/core/configoptions.h"
#include "hart/core/engine.h"
#include "hart/core/entity.h"
#include "hart/core/input.h"
#include "hart/core/objectfactory.h"
#include "hart/core/resourcemanager.h"
#include "hart/core/taskgraph.h"
#include "hart/core/utf8.h"

#include "hart/lfds/lfds.h"

#include "hart/render/material.h"
#include "hart/render/program.h"
#include "hart/render/render.h"
#include "hart/render/shader.h"
#include "hart/render/technique.h"
#include "hart/render/texture.h"
#include "hart/render/vertexdecl.h"
