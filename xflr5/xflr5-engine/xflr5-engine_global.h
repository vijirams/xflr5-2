#ifndef XFLR5ENGINE_GLOBAL_H
#define XFLR5ENGINE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(XFLR5ENGINE_LIBRARY)
#  define XFLR5ENGINELIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define XFLR5ENGINELIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // XFLR5ENGINE_GLOBAL_H
