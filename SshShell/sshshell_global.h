#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(SSHSHELL_LIB)
#  define SSHSHELL_EXPORT Q_DECL_EXPORT
# else
#  define SSHSHELL_EXPORT Q_DECL_IMPORT
# endif
#else
# define SSHSHELL_EXPORT
#endif
