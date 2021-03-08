/****************************************************************************

    gl3dWingView Class
    Copyright (C) 2016 Andre Deperrois

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#ifndef GL3DWINGVIEW_H
#define GL3DWINGVIEW_H

#include <viewwidgets/glwidgets/gl3dview.h>

class Wing;
class GL3dWingDlg;

class gl3dWingView : public gl3dView
{
    friend class GL3dWingDlg;

public:
    gl3dWingView(QWidget *pParent = nullptr);
    void setWing(Wing const*pWing);
    void glMakeWingSectionHighlight(Wing const *pWing, int iSectionHighLight, bool bRightSide);

private:
    void glRenderView();

    void set3DRotationCenter(QPoint point);
    void glMake3dObjects();

public slots:
    void on3DReset();

private:
    Wing const*m_pWing;
    GL3dWingDlg *m_pGL3dWingDlg;

    bool m_bResetglWing;
    bool m_bResetglSectionHighlight;

};

#endif // GL3DWINGVIEW_H
