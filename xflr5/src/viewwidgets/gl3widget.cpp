/****************************************************************************

	GL3Widget Class
	Copyright (C) 2016 Andre Deperrois adeperrois@xflr5.com

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
#include <QApplication>
#include <QOpenGLShaderProgram>
#include <QOpenGLPaintDevice>
#include <QMouseEvent>
#include <QProgressDialog>
#include "gl3widget.h"
#include "CVector.h"
#include "Settings.h"
#include "mainframe.h"
#include <miarex/Miarex.h>
#include <Body.h>
#include <Wing.h>
#include <Plane.h>
#include <miarex/Objects3D.h>
#include <miarex/design/GL3dBodyDlg.h>
#include <miarex/design/GL3dWingDlg.h>
#include <miarex/design/EditBodyDlg.h>
#include <miarex/design/EditPlaneDlg.h>
#include <miarex/analysis/LLTAnalysis.h>
#include <miarex/view/GL3DScales.h>
#include <misc/W3dPrefsDlg.h>
#include <misc/Settings.h>
#include <objects/WPolar.h>
#include <objects/Surface.h>
#include <params.h>


#define CHORDPOINTS 29


#define NH  39
#define NX  59



void *GL3Widget::s_pMiarex;
void *GL3Widget::s_pMainFrame;
GLLightDlg *GL3Widget::s_pglLightDlg = NULL;


GL3Widget::GL3Widget(QWidget *pParent) : QOpenGLWidget(pParent)
{
	setAutoFillBackground(false);
	setMouseTracking(true);
	setCursor(Qt::CrossCursor);

	m_pParent = pParent;

	m_clearColor = Qt::black;
	m_glScaled = m_glScaledRef = 1.0f;

	m_bArcball = m_bCrossPoint = false;

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		m_WingIndicesArray[iw] = NULL;
	}
	m_BodyIndicesArray = NULL;
	m_SphereIndicesArray = NULL;
	m_WingMeshIndicesArray = NULL;

	m_pLeftBodyTexture = m_pRightBodyTexture= NULL;
	m_pWingTopLeftTexture = m_pWingTopRightTexture = NULL;
	m_pWingBotLeftTexture = m_pWingBotRightTexture = NULL;

	m_iView = XFLR5::GLMIAREXVIEW;

	m_bOutline    = true;
	m_bSurfaces   = true;
	m_bVLMPanels  = false;
	m_bAxes       = true;
	m_bShowMasses = false;
	m_bFoilNames  = false;

	m_iBodyElems = 0;
	for(int iw=0; iw<MAXWINGS; iw++) m_iWingElems[iw]=0;
	m_iWingMeshElems = 0;

	m_NStreamLines = 0;
	m_nHighlightLines = m_HighlightLineSize = 0;

	m_glViewportTrans.x  = 0.0;
	m_glViewportTrans.y  = 0.0;
	m_glViewportTrans.z  = 0.0;

	m_ClipPlanePos  = 5.0;
	m_glScaled      = 1.0;
	m_glScaledRef   = 1.0;

	m_bTrans = false;

	m_LastPoint.setX(0);
	m_LastPoint.setY(0);

	m_PixTextOverlay = QPixmap(107, 97);
	m_PixTextOverlay.fill(Qt::transparent);

	memset(MatIn,  0, 16*sizeof(double));
	memset(MatOut, 0, 16*sizeof(double));

#ifdef QT_DEBUG
	printFormat(format());
#endif
}


void GL3Widget::printFormat(const QSurfaceFormat &format)
{
	qDebug()<<tr("OpenGL version: %1.%2").arg(format.majorVersion()).arg(format.minorVersion());

	switch (format.profile()) {
		case QSurfaceFormat::NoProfile:
				qDebug()<<"No Profile";
			break;
		case QSurfaceFormat::CoreProfile:
				qDebug()<<"Core Profile";
			break;
		case QSurfaceFormat::CompatibilityProfile:
				qDebug()<<"Compatibility Profile";
			break;
		default:
			break;
	}
	switch (format.renderableType())
	{
		case QSurfaceFormat::DefaultRenderableType:
				qDebug()<<"DefaultRenderableType: The default, unspecified rendering method";
			break;
		case QSurfaceFormat::OpenGL:
				qDebug()<<"OpenGL: Desktop OpenGL rendering";
			break;
		case QSurfaceFormat::OpenGLES:
				qDebug()<<"OpenGLES: OpenGL ES 2.0 rendering";
			break;
		case QSurfaceFormat::OpenVG:
				qDebug()<<"OpenVG: Open Vector Graphics rendering";
			break;
		default:
			break;
	}
	qDebug()<<tr("Depth buffer size: %1").arg(QString::number(format.depthBufferSize()));
	qDebug()<<tr("Stencil buffer size: %1").arg(QString::number(format.stencilBufferSize()));
	qDebug()<<tr("Samples: %1").arg(QString::number(format.samples()));
	qDebug()<<tr("Red buffer size: %1").arg(QString::number(format.redBufferSize()));
	qDebug()<<tr("Green buffer size: %1").arg(QString::number(format.greenBufferSize()));
	qDebug()<<tr("Blue buffer size: %1").arg(QString::number(format.blueBufferSize()));
	qDebug()<<tr("Alpha buffer size: %1").arg(QString::number(format.alphaBufferSize()));
	qDebug()<<tr("Swap interval: %1").arg(QString::number(format.swapInterval()));
}


GL3Widget::~GL3Widget()
{
	for(int iWing=0; iWing<MAXWINGS; iWing++)
	{
		if(m_WingIndicesArray[iWing]) delete [] m_WingIndicesArray[iWing];
	}

	if(m_BodyIndicesArray)     delete[] m_BodyIndicesArray;
	if(m_SphereIndicesArray)   delete[] m_SphereIndicesArray;
	if(m_WingMeshIndicesArray) delete[] m_WingMeshIndicesArray;

	// release all OpenGL resources.
	makeCurrent();
	m_vboArcBall.destroy();
	m_vboArcPoint.destroy();
	m_vboSphere.destroy();
	m_vboBody.destroy();
	m_vboMesh.destroy();
	m_vboWingMesh.destroy();
	m_vboPanelCp.destroy();

	for(int iWing=0; iWing<MAXWINGS; iWing++)
	{
		m_vboWing[iWing].destroy();
		m_vboLift[iWing].destroy();
		m_vboICd[iWing].destroy();
		m_vboVCd[iWing].destroy();
		m_vboTransitions[iWing].destroy();
		m_vboDownwash[iWing].destroy();
	}

	if(m_pLeftBodyTexture)     delete m_pLeftBodyTexture;
	if(m_pRightBodyTexture)    delete m_pRightBodyTexture;
	if(m_pWingBotLeftTexture)  delete m_pWingBotLeftTexture;
	if(m_pWingTopLeftTexture)  delete m_pWingTopLeftTexture;
	if(m_pWingBotRightTexture) delete m_pWingBotRightTexture;
	if(m_pWingTopRightTexture) delete m_pWingTopRightTexture;

	doneCurrent();
}


QSize GL3Widget::sizeHint() const
{
	return QSize(640, 480);
}


QSize GL3Widget::minimumSizeHint() const
{
	return QSize(250, 200);
}


/**
* The user has modified the position of the clip plane in the 3D view
*@param pos the new z position in viewport coordinates of the clipping plane
*/
void GL3Widget::onClipPlane(int pos)
{
	double planepos =  (double)pos/100.0;
	m_ClipPlanePos = sinh(planepos) * 0.5;
	update();
}


void GL3Widget::on3DIso()
{
	m_ArcBall.ab_quat[0]	= -0.65987748f;
	m_ArcBall.ab_quat[1]	=  0.38526487f;
	m_ArcBall.ab_quat[2]	= -0.64508355f;
	m_ArcBall.ab_quat[3]	=  0.0f;
	m_ArcBall.ab_quat[4]	= -0.75137258f;
	m_ArcBall.ab_quat[5]	= -0.33720365f;
	m_ArcBall.ab_quat[6]	=  0.56721509f;
	m_ArcBall.ab_quat[7]	=  0.0f;
	m_ArcBall.ab_quat[8]	=  0.000f;
	m_ArcBall.ab_quat[9]	=  0.85899049f;
	m_ArcBall.ab_quat[10]	=  0.51199043f;
	m_ArcBall.ab_quat[11]	=  0.0f;
	m_ArcBall.ab_quat[12]	=  0.0f;
	m_ArcBall.ab_quat[13]	=  0.0f;
	m_ArcBall.ab_quat[14]	=  0.0f;
	m_ArcBall.ab_quat[15]	=  1.0f;

	reset3DRotationCenter();
	emit(viewModified());
	update();
}



void GL3Widget::on3DTop()
{
	m_ArcBall.setQuat(sqrt(2.0)/2.0, 0.0, 0.0, -sqrt(2.0)/2.0);
	reset3DRotationCenter();
	emit(viewModified());
	update();
}


void GL3Widget::on3DLeft()
{
	m_ArcBall.setQuat(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0, 0.0);// rotate by 90° around x
	reset3DRotationCenter();
	emit(viewModified());
	update();
}


void GL3Widget::on3DFront()
{
	Quaternion Qt1(sqrt(2.0)/2.0, 0.0,           -sqrt(2.0)/2.0, 0.0);// rotate by 90° around y
	Quaternion Qt2(sqrt(2.0)/2.0, -sqrt(2.0)/2.0, 0.0,           0.0);// rotate by 90° around x

	m_ArcBall.setQuat(Qt1 * Qt2);
	reset3DRotationCenter();
	emit(viewModified());
	update();
}



void GL3Widget::onSurfaces(bool bChecked)
{
	m_bSurfaces = bChecked;
	update();
}


void GL3Widget::onOutline(bool bChecked)
{
	m_bOutline = bChecked;
	update();
}


void GL3Widget::onPanels(bool bChecked)
{
	m_bVLMPanels = bChecked;
	update();
}

void GL3Widget::onAxes(bool bChecked)
{
	m_bAxes = bChecked;
	update();
}


void GL3Widget::onFoilNames(bool bChecked)
{
	m_bFoilNames = bChecked;
	update();
}


void GL3Widget::onShowMasses(bool bChecked)
{
	m_bShowMasses = bChecked;
	update();
}


void GL3Widget::mousePressEvent(QMouseEvent *event)
{
	QPoint point(event->pos().x(), event->pos().y());

	bool bCtrl = false;
	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

	setFocus();

	if (event->buttons() & Qt::MidButton)
	{
		m_bArcball = true;
		CVector real;
		QPoint pt(event->pos().x(), event->pos().y());
		screenToViewport(pt, real);
		m_ArcBall.start(real.x, real.y);
		m_bCrossPoint = true;

		reset3DRotationCenter();
		update();
	}
	else if (event->buttons() & Qt::LeftButton)
	{
		CVector real;
		QPoint pt(point.x(), point.y());
		screenToViewport(pt, real);
		m_ArcBall.start(real.x, real.y);
		m_bCrossPoint = true;
		reset3DRotationCenter();
		if (!bCtrl)
		{
			m_bTrans = true;
			setCursor(Qt::ClosedHandCursor);
		}
		else
		{
			m_bTrans=false;
			m_bArcball = true;
		}
		update();
	}

	m_LastPoint = point;
	QVector4D  x1(1.0, 0.0, 0.0, 1.0);
	QVector4D  y1(0.0, 1.0, 0.0, 1.0);
	QVector4D  z1(0.0, 0.0, 1.0, 1.0);

}


QPoint GL3Widget::worldToScreen(CVector v)
{
	QVector4D v4(v.x, v.y, v.z, 1.0);
	QVector4D vS = m_pvmMatrix * v4;
	return QPoint((int)((vS.x()+1.0)*width()/2), (int)((1.0-vS.y())*height()/2));
}


QPoint GL3Widget::worldToScreen(QVector4D v4)
{
	QVector4D vS = m_pvmMatrix * v4;
	return QPoint((int)((vS.x()+1.0)*width()/2), (int)((1.0-vS.y())*height()/2));
}



/**
*Overrides the mouseDoubleClickEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void GL3Widget::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(m_iView == XFLR5::GLMIAREXVIEW)
	{
//		QMiarex* pMiarex = (QMiarex*)s_pMiarex;
		set3DRotationCenter(event->pos());
	}
	else if(m_iView == XFLR5::GLBODYVIEW)
	{
//		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		set3DRotationCenter(event->pos());
	}
	else if(m_iView == XFLR5::GLWINGVIEW)
	{
//		GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
		set3DRotationCenter(event->pos());
	}
	else if(m_iView == XFLR5::GLPLANEVIEW)
	{
//		ViewObjectDlg *pDlg = (ViewObjectDlg*)m_pParent;
		set3DRotationCenter(event->pos());
	}
	else if(m_iView == XFLR5::GLEDITBODYVIEW)
	{
//		ViewObjectDlg *pDlg = (ViewObjectDlg*)m_pParent;
		set3DRotationCenter(event->pos());
	}
}



void GL3Widget::mouseMoveEvent(QMouseEvent *event)
{
	QPoint point(event->pos().x(), event->pos().y());
	CVector Real;

	QPoint Delta(point.x()-m_LastPoint.x(), point.y()-m_LastPoint.y());
	screenToViewport(point, Real);

//	if(!hasFocus()) setFocus();

	bool bCtrl = false;

	if (event->modifiers() & Qt::ControlModifier) bCtrl =true;
	if (event->buttons()   & Qt::LeftButton)
	{
		if(bCtrl)
		{
			//rotate
			m_ArcBall.move(Real.x, Real.y);
			update();
		}
		else if(m_bTrans)
		{
			//translate

			int side = qMax(geometry().width(), geometry().height());

			m_glViewportTrans.x += (GLfloat)(Delta.x()*2.0/m_glScaled/side);
			m_glViewportTrans.y += (GLfloat)(Delta.y()*2.0/m_glScaled/side);

			m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
			m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
			m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;

			update();

		}
	}

	else if (event->buttons() & Qt::MidButton)
	{
		m_ArcBall.move(Real.x, Real.y);
		update();
	}
	else if(event->modifiers().testFlag(Qt::AltModifier))
	{
		double zoomFactor=1.0;

		if(point.y()-m_LastPoint.y()<0) zoomFactor = 1./1.06;
		else                            zoomFactor = 1.06;

		if(m_iView == XFLR5::GLMIAREXVIEW)
		{
			m_glScaled *= zoomFactor;
			update();
		}
	}
	m_LastPoint = point;
}



void GL3Widget::mouseReleaseEvent(QMouseEvent * event )
{
	setCursor(Qt::CrossCursor);

	m_bTrans      = false;
	m_bDragPoint  = false;
	m_bArcball    = false;
	m_bCrossPoint = false;

//	We need to re-calculate the translation vector
	int i,j;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			MatIn[i][j] =  m_ArcBall.ab_quat[i*4+j];

	glInverseMatrix();
	m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
	m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
	m_glViewportTrans.z =  (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);


	update();
	event->accept();
}



/**
*Overrides the wheelEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void GL3Widget::wheelEvent(QWheelEvent *event)
{
	double zoomFactor=1.0;
	if(event->delta()>0)
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1./1.06;
		else                          zoomFactor = 1.06;
	}
	else
	{
		if(!Settings::s_bReverseZoom) zoomFactor = 1.06;
		else                          zoomFactor = 1./1.06;
	}
	m_glScaled *= zoomFactor;
	update();

	QPoint glPoint(event->pos().x() + geometry().x(), event->pos().y()+geometry().y());

	if(geometry().contains(glPoint))
	{
		setFocus();	//The mouse button has been wheeled
		m_glScaled *= zoomFactor;
	}
	update();
}



/**
*Overrides the contextMenuEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void GL3Widget::contextMenuEvent (QContextMenuEvent * event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	QPoint ScreenPt = event->globalPos();
	m_bArcball = false;
	update();
	if(m_iView == XFLR5::GLMIAREXVIEW)
	{
		QMiarex *pMiarex = (QMiarex *)s_pMiarex;
		if (pMiarex->m_iView==XFLR5::W3DVIEW)
		{
			if(pMiarex->m_pCurWPolar && pMiarex->m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
				pMainFrame->m_pW3DStabCtxMenu->exec(ScreenPt);
			else pMainFrame->m_pW3DCtxMenu->exec(ScreenPt);
		}
	}
	else  if(m_iView == XFLR5::GLBODYVIEW)
	{
		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
		pDlg->showContextMenu(event);
	}
	else if(m_iView == XFLR5::GLWINGVIEW)
	{
//		GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
//		pDlg->ShowContextMenu(event);
	}
}

/**
*Overrides the keyPressEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void GL3Widget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Control:
		{
			m_bArcball = true;
			update();
			break;
		}
		case Qt::Key_R:
		{
			on3DReset();
			event->accept();
			break;
		}
		case Qt::Key_X:
		{
			break;
		}
		default:
			event->ignore();
	}
}


/**
*Overrides the keyReleaseEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void GL3Widget::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Control:
		{
			m_bArcball = false;
			update();
			break;
		}

		default:
			event->ignore();
	}
}


void GL3Widget::on3DReset()
{
//	if(m_iView == XFLR5::GLMIAREXVIEW || m_iView == XFLR5::GLWINGVIEW || m_iView == XFLR5::GLPLANEVIEW || m_iView == XFLR5::GLEDITBODYVIEW)

/*	if(m_iView == XFLR5::GLMIAREXVIEW)
	{
		QMiarex *pMiarex = (QMiarex *)s_pMiarex;
		pMiarex->m_bIs3DScaleSet = false;
		pMiarex->set3DScale();
	}*/

	m_glViewportTrans.set(0.0, 0.0, 0.0);
	reset3DRotationCenter();
	update();
}


void GL3Widget::reset3DRotationCenter()
{
	//adjust the new rotation center after a translation or a rotation

	int i,j;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			MatOut[i][j] =  m_ArcBall.ab_quat[i*4+j];

	m_glRotCenter.x = MatOut[0][0]*(m_glViewportTrans.x) + MatOut[0][1]*(-m_glViewportTrans.y) + MatOut[0][2]*m_glViewportTrans.z;
	m_glRotCenter.y = MatOut[1][0]*(m_glViewportTrans.x) + MatOut[1][1]*(-m_glViewportTrans.y) + MatOut[1][2]*m_glViewportTrans.z;
	m_glRotCenter.z = MatOut[2][0]*(m_glViewportTrans.x) + MatOut[2][1]*(-m_glViewportTrans.y) + MatOut[2][2]*m_glViewportTrans.z;
}


void GL3Widget::set3DRotationCenter(QPoint point)
{
	//adjusts the new rotation center after the user has picked a point on the screen
	//finds the closest panel under the point,
	//and changes the rotation vector and viewport translation
	CVector I, A, B, AA, BB, PP, U;

	screenToViewport(point, B);
	B.z = -1.0;
	A.set(B.x, B.y, +1.0);

	viewportToWorld(A, AA);
	viewportToWorld(B, BB);

	U.set(BB.x-AA.x, BB.y-AA.y, BB.z-AA.z);
	U.normalize();

	bool bIntersect = false;

	if(m_iView == XFLR5::GLWINGVIEW)
	{
		GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
		if(pDlg->intersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.set(I);
		}
	}
	else if(m_iView == XFLR5::GLPLANEVIEW)
	{
		EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
		if(pDlg->intersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.set(I);
		}
	}
	else if(m_iView == XFLR5::GLEDITBODYVIEW)
	{
		EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
		if(pDlg->intersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.set(I);
		}
	}
	else if(m_iView == XFLR5::GLMIAREXVIEW)
	{
		QMiarex *pMiarex = (QMiarex*)s_pMiarex;
		if(pMiarex->intersectObject(AA, U, I))
		{
			bIntersect = true;
			PP.set(I);
		}
	}

	if(bIntersect)
	{
//		instantaneous visual transition
//		m_glRotCenter -= PP * m_glScaled;

//		smooth visual transition
		glInverseMatrix();

		U.x = (-PP.x -m_glRotCenter.x)/30.0;
		U.y = (-PP.y -m_glRotCenter.y)/30.0;
		U.z = (-PP.z -m_glRotCenter.z)/30.0;

		for(int i=0; i<30; i++)
		{
			m_glRotCenter +=U;
			m_glViewportTrans.x =  (MatOut[0][0]*m_glRotCenter.x + MatOut[0][1]*m_glRotCenter.y + MatOut[0][2]*m_glRotCenter.z);
			m_glViewportTrans.y = -(MatOut[1][0]*m_glRotCenter.x + MatOut[1][1]*m_glRotCenter.y + MatOut[1][2]*m_glRotCenter.z);
			m_glViewportTrans.z=   (MatOut[2][0]*m_glRotCenter.x + MatOut[2][1]*m_glRotCenter.y + MatOut[2][2]*m_glRotCenter.z);

			update();
		}
	}
}



void GL3Widget::glInverseMatrix()
{
	//Step 1. Transpose the 3x3 rotation portion of the 4x4 matrix to get the inverse rotation
	int i,j;

	for(i=0 ; i<3; i++)
	{
		for(j=0; j<3; j++)
		{
			MatOut[j][i] = MatIn[i][j];
		}
	}
}


/**
*Converts screen coordinates to OpenGL Viewport coordinates.
*@param point the screen coordinates.
*@param real the viewport coordinates.
*/
void GL3Widget::screenToViewport(QPoint const &point, CVector &real)
{
	double h2, w2;
	h2 = (double)geometry().height() /2.0;
	w2 = (double)geometry().width()  /2.0;

	if(w2>h2)
	{
		real.x =  ((double)point.x() - w2) / w2;
		real.y = -((double)point.y() - h2) / w2;
	}
	else
	{
		real.x =  ((double)point.x() - w2) / h2;
		real.y = -((double)point.y() - h2) / h2;
	}
}



/**
*Converts OpenGL Viewport coordinates to screen coordinates
*@param real the viewport coordinates.
*@param point the screen coordinates.
*/
void GL3Widget::viewportToScreen(CVector const &real, QPoint &point)
{
	double dx, dy, h2, w2;

	h2 = m_GLViewRect.height() /2.0;
	w2 = m_GLViewRect.width()  /2.0;

	dx = ( real.x + w2)/2.0;
	dy = (-real.y + h2)/2.0;

	if(w2>h2)
	{
		point.setX((int)(dx * (double)geometry().width()));
		point.setY((int)(dy * (double)geometry().width()));
	}
	else
	{
		point.setX((int)(dx * (double)geometry().height()));
		point.setY((int)(dy * (double)geometry().height()));
	}
}


QVector4D GL3Widget::worldToViewport(CVector v)
{
	QVector4D v4(v.x, v.y, v.z, 1.0);
	return m_pvmMatrix * v4;
}


void GL3Widget::viewportToWorld(CVector vp, CVector &w)
{
	//un-translate
	vp.x += - m_glViewportTrans.x*m_glScaled;
	vp.y += + m_glViewportTrans.y*m_glScaled;

	//un-scale
	vp.x *= 1.0/m_glScaled;
	vp.y *= 1.0/m_glScaled;
	vp.z *= 1.0/m_glScaled;


	//un-rotate
	w.x = m_ArcBall.ab_quat[0]*vp.x + m_ArcBall.ab_quat[1]*vp.y + m_ArcBall.ab_quat[2]*vp.z;
	w.y = m_ArcBall.ab_quat[4]*vp.x + m_ArcBall.ab_quat[5]*vp.y + m_ArcBall.ab_quat[6]*vp.z;
	w.z = m_ArcBall.ab_quat[8]*vp.x + m_ArcBall.ab_quat[9]*vp.y + m_ArcBall.ab_quat[10]*vp.z;
}



void GL3Widget::glRenderText(double x, double y, double z, const QString & str, QColor textColor)
{
	QPoint point;

	if(z>m_ClipPlanePos) return;

	point = worldToScreen(CVector(x,y,z));
	QPainter paint(&m_PixTextOverlay);
	paint.save();
	QPen textPen(textColor);
	paint.setPen(textPen);
	paint.drawText(point, str);
	paint.restore();
}




void GL3Widget::glRenderText(int x, int y, const QString & str, QColor textColor)
{
	QPainter paint(&m_PixTextOverlay);
	paint.save();
	QPen textPen(textColor);
	paint.setPen(textPen);

	paint.drawText(x,y, str);
	paint.restore();
}


/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void GL3Widget::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);


	double w, h, s;
	w = (double)width;
	h = (double)height;
	s = 1.0;


	if(w>h)	m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
	else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

	m_PixTextOverlay = m_PixTextOverlay.scaled(rect().size());
	m_PixTextOverlay.fill(Qt::transparent);


	if(m_iView == XFLR5::GLMIAREXVIEW)
	{
		if(s_pMiarex)
		{
			QMiarex* pMiarex = (QMiarex*)s_pMiarex;
	//		pMiarex->m_ArcBall.GetMatrix();
			pMiarex->m_bIs3DScaleSet = false;
			pMiarex->m_bResetTextLegend = true;
			pMiarex->set3DScale();
		}
	}
	else if(m_iView == XFLR5::GLWINGVIEW)
	{
	}
	else if(m_iView == XFLR5::GLBODYVIEW)
	{
	}
	else if(m_iView == XFLR5::GLPLANEVIEW)
	{
		if(m_pParent)
		{
			EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
			pDlg->resize3DView();
		}
	}
}



void GL3Widget::getGLError()
{
	switch(glGetError())
	{
		case GL_NO_ERROR:
			qDebug()<<"No error has been recorded. The value of this symbolic constant is guaranteed to be 0.";
			break;

		case GL_INVALID_ENUM:
			qDebug()<<"An unacceptable value is specified for an enumerated argument. "
					  "The offending command is ignored and has no other side effect than to set the error flag.";
			break;

		case GL_INVALID_VALUE:
			qDebug()<<"A numeric argument is out of range. The offending command is ignored and has no other"
					  " side effect than to set the error flag.";
			break;

		case GL_INVALID_OPERATION:
			qDebug()<<"The specified operation is not allowed in the current state. The offending command is"
					  " ignored and has no other side effect than to set the error flag.";
			break;

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			qDebug()<<"The command is trying to render to or read from the framebuffer while the currently "
					  "bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus "
					  "is not GL_FRAMEBUFFER_COMPLETE). The offending command is ignored and has no other side effect than "
					  "to set the error flag.";
			break;

		case GL_OUT_OF_MEMORY:
			qDebug()<<"There is not enough memory left to execute the command. The state of the GL is "
					  "undefined, except for the state of the error flags, after this error is recorded.";
			break;

		case GL_STACK_UNDERFLOW:
			qDebug()<<"An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;

		case GL_STACK_OVERFLOW:
			qDebug()<<"An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
	}
}


static GLfloat const x_axis[] = {
	-1.0f, 0.0f, 0.0f,
	 1.0f, 0.0f, 0.0f,
	 1.0f,   0.0f,   0.0f,
	 0.95f,  0.015f, 0.015f,
	 1.0f,  0.0f,    0.0f,
	 0.95f,-0.015f,-0.015f
};

static GLfloat const y_axis[] = {
	  0.0f,    -1.0f,    0.0f,
	  0.0f,     1.0f,    0.0f,
	  0.f,      1.0f,    0.0f,
	  0.015f,   0.95f,   0.015f,
	  0.f,      1.0f,    0.0f,
	 -0.015f,   0.95f,  -0.015f
};

static GLfloat const z_axis[] = {
	 0.0f,    0.0f,   -1.0f,
	 0.0f,    0.0f,    1.0f,
	 0.0f,    0.0f,    1.0f,
	 0.015f,  0.015f,  0.95f,
	 0.0f,    0.0f,    1.0f,
	-0.015f, -0.015f,  0.95f
};



void GL3Widget::setClearColor(const QColor &color)
{
	m_clearColor = color;
	update();
}


#define NUMANGLES     10
#define NUMCIRCLES     6
#define NUMPERIM      35
#define NUMARCPOINTS  10




/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view
*@param GLScale the overall scaling factor for the view
*/
void GL3Widget::glMakeArcPoint()
{
	float GLScale = 1.0f;
	int row, col;
	double Radius=0.1, lat_incr, phi, theta;
	CVector eye(0.0, 0.0, 1.0);
	CVector up(0.0, 1.0, 0.0);
	m_ArcBall.setZoom(0.45, eye, up);

	Radius = m_ArcBall.ab_sphere;

	int iv=0;

	float *arcPointVertexArray = new float[NUMARCPOINTS*2*2*3];

	//ARCPOINT
	lat_incr = 30.0 / NUMARCPOINTS;

	phi = 0.0* PI/180.0;//longitude
	for (row = -NUMARCPOINTS; row < NUMARCPOINTS; row++)
	{
		theta = (0.0+ row * lat_incr) * PI/180.0;
		arcPointVertexArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius*sin(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
	}

	theta = 0.0* PI/180.0;
	for(col=-NUMARCPOINTS; col<NUMARCPOINTS; col++)
	{
		phi = (0.0 + (double)col*30.0/NUMARCPOINTS) * PI/180.0;
		arcPointVertexArray[iv++] = Radius * cos(phi) * cos(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius * sin(theta)*GLScale;
		arcPointVertexArray[iv++] = Radius * sin(phi) * cos(theta)*GLScale;
	}

	m_vboArcPoint.destroy();
	m_vboArcPoint.create();
	m_vboArcPoint.bind();
	m_vboArcPoint.allocate(arcPointVertexArray, iv * sizeof(GLfloat));
	m_vboArcPoint.release();

	delete [] arcPointVertexArray;
}



void GL3Widget::glMakeBody(Body *pBody)
{
	CVector m_T[(NX+1)*(NH+1)]; //temporary points to save calculation times for body NURBS surfaces
	CVector TALB, LATB;
	int j, k, l, p;
	double v;

	if(!pBody) return;

	CVector Point;
	double hinc, u;
	CVector N;

	if(m_pLeftBodyTexture)  delete m_pLeftBodyTexture;
	if(m_pRightBodyTexture) delete m_pRightBodyTexture;
	m_pLeftBodyTexture  = new QOpenGLTexture(QImage(QString(":/images/body_left.png")));
	m_pRightBodyTexture = new QOpenGLTexture(QImage(QString(":/images/body_right.png")));


	//vertices array size:
	// surface:
	//     (NX+1)*(NH+1) : from 0 to NX, and from 0 to NH
	//     x2 : 2 sides
	// outline:
	//     frameSize()*(NH+1)*2 : frames
	//     (NX+1) + (NX+1)      : top and bottom lines
	//
	// x8 : 3 vertices components, 3 normal components, 2 texture componenents
	int bodyVertexSize;
	bodyVertexSize  =   (NX+1)*(NH+1) *2             // side surfaces
					  + pBody->frameCount()*(NH+1)*2 // frames
					  + (NX+1)                       // top outline
					  + (NX+1);                      // bot outline

	bodyVertexSize *= 8; // 3 vertex components, 3 normal components, 2 uv components

	float *bodyVertexArray = new float[bodyVertexSize];

	p = 0;
	for (k=0; k<=NX; k++)
	{
		u = (double)k / (double)NX;
		for (l=0; l<=NH; l++)
		{
			v = (double)l / (double)NH;
			pBody->getPoint(u,  v, true, m_T[p]);
			p++;
		}
	}

	int iv=0; //index of vertex components

	//right side first;
	p=0;
	for (k=0; k<=NX; k++)
	{
		for (l=0; l<=NH; l++)
		{
			bodyVertexArray[iv++] = m_T[p].x;
			bodyVertexArray[iv++] = m_T[p].y;
			bodyVertexArray[iv++] = m_T[p].z;

			if(k==0)       N.set(-1.0, 0.0, 0.0);
			else if(k==NX) N.set(1.0, 0.0, 0.0);
			else if(l==0)  N.set(0.0, 0.0, 1.0);
			else if(l==NH) N.set(0.0,0.0, -1.0);
			else
			{
				LATB = m_T[p+NH+1] - m_T[p+1];     //	LATB = TB - LA;
				TALB = m_T[p]  - m_T[p+NH+2];      //	TALB = LB - TA;
				N = TALB * LATB;
				N.normalize();
			}

			bodyVertexArray[iv++] = N.x;
			bodyVertexArray[iv++] = N.y;
			bodyVertexArray[iv++] = N.z;

			bodyVertexArray[iv++] = (float)(NX-k)/(float)NX;
			bodyVertexArray[iv++] = (float)l/(float)NH;
			p++;
		}
	}


	//left side next;
	p=0;
	for (k=0; k<=NX; k++)
	{
		for (l=0; l<=NH; l++)
		{
			bodyVertexArray[iv++] =  m_T[p].x;
			bodyVertexArray[iv++] = -m_T[p].y;
			bodyVertexArray[iv++] =  m_T[p].z;

			if(k==0) N.set(-1.0, 0.0, 0.0);
			else if(k==NX) N.set(1.0, 0.0, 0.0);
			else if(l==0)  N.set(0.0, 0.0, 1.0);
			else if(l==NH) N.set(0.0,0.0, -1.0);
			else
			{
				LATB = m_T[p+NH+1] - m_T[p+1];     //	LATB = TB - LA;
				TALB = m_T[p]  - m_T[p+NH+2];      //	TALB = LB - TA;
				N = TALB * LATB;
				N.normalize();
			}
			bodyVertexArray[iv++] = N.x;
			bodyVertexArray[iv++] = N.y;
			bodyVertexArray[iv++] = N.z;

			bodyVertexArray[iv++] = (float)k/(float)NX;
			bodyVertexArray[iv++] = (float)l/(float)NH;
			p++;
		}
	}

	//OUTLINE
	hinc=1./(double)NH;
	u=0.0; v = 0.0;

	// frames : frameCount() x (NH+1)
	for (int iFr=0; iFr<pBody->frameCount(); iFr++)
	{
		u = pBody->getu(pBody->frame(iFr)->m_Position.x);
		for (j=0; j<=NH; j++)
		{
			v = (double)j*hinc;
			pBody->getPoint(u,v,true, Point);
			bodyVertexArray[iv++] = Point.x;
			bodyVertexArray[iv++] = Point.y;
			bodyVertexArray[iv++] = Point.z;

			N = CVector(0.0, Point.y, Point.z);
			N.normalize();
			bodyVertexArray[iv++] = N.x;
			bodyVertexArray[iv++] = N.y;
			bodyVertexArray[iv++] = N.z;

			bodyVertexArray[iv++] = u;
			bodyVertexArray[iv++] = v;
		}

		for (j=NH; j>=0; j--)
		{
			v = (double)j*hinc;
			pBody->getPoint(u,v,false, Point);
			bodyVertexArray[iv++] = Point.x;
			bodyVertexArray[iv++] = Point.y;
			bodyVertexArray[iv++] = Point.z;
			N = CVector(0.0, Point.y, Point.z);
			N.normalize();
			bodyVertexArray[iv++] = N.x;
			bodyVertexArray[iv++] = N.y;
			bodyVertexArray[iv++] = N.z;

			bodyVertexArray[iv++] = u;
			bodyVertexArray[iv++] = v;
		}
	}

	//top line: NX+1
	v = 0.0;
	for (int iu=0; iu<=NX; iu++)
	{
		pBody->getPoint((double)iu/(double)NX,v, true, Point);
		bodyVertexArray[iv++] = Point.x;
		bodyVertexArray[iv++] = Point.y;
		bodyVertexArray[iv++] = Point.z;

		bodyVertexArray[iv++] = N.x;
		bodyVertexArray[iv++] = N.y;
		bodyVertexArray[iv++] = N.z;

		bodyVertexArray[iv++] = (float)iu/(float)NX;
		bodyVertexArray[iv++] = v;
	}

	//bottom line: NX+1
	v = 1.0;
	for (int iu=0; iu<=NX; iu++)
	{
		pBody->getPoint((double)iu/(double)NX,v, true, Point);
		bodyVertexArray[iv++] = Point.x;
		bodyVertexArray[iv++] = Point.y;
		bodyVertexArray[iv++] = Point.z;
		bodyVertexArray[iv++] = N.x;
		bodyVertexArray[iv++] = N.y;
		bodyVertexArray[iv++] = N.z;

		bodyVertexArray[iv++] = (float)iu/(float)NX;
		bodyVertexArray[iv++] = v;
	}


	//Create triangles
	//  indices array size:
	//    NX*NH
	//    2 triangles per/quad
	//    3 indices/triangle
	//    2 sides
	if(m_BodyIndicesArray) delete[] m_BodyIndicesArray;
	m_BodyIndicesArray = new unsigned short[NX*NH*2*3*2];

	int ii=0;
	int nV;

	//left side;
	for (k=0; k<NX; k++)
	{
		for (l=0; l<NH; l++)
		{
			nV = k*(NH+1)+l; // id of the vertex at the bottom left of the quad
			//first triangle
			m_BodyIndicesArray[ii]   = nV;
			m_BodyIndicesArray[ii+1] = nV+NH+1;
			m_BodyIndicesArray[ii+2] = nV+1;

			//second triangle
			m_BodyIndicesArray[ii+3] = nV+NH+1;
			m_BodyIndicesArray[ii+4] = nV+1;
			m_BodyIndicesArray[ii+5] = nV+NH+1+1;
			ii += 6;
		}
	}

	//right side
	for (k=0; k<NX; k++)
	{
		for (l=0; l<NH; l++)
		{
			nV = (NX+1)*(NH+1) + k*(NH+1)+l; // id of the vertex at the bottom left of the quad
			//first triangle
			m_BodyIndicesArray[ii]   = nV;
			m_BodyIndicesArray[ii+1] = nV+NH+1;
			m_BodyIndicesArray[ii+2] = nV+1;

			//second triangle
			m_BodyIndicesArray[ii+3] = nV+NH+1;
			m_BodyIndicesArray[ii+4] = nV+1;
			m_BodyIndicesArray[ii+5] = nV+NH+1+1;
			ii += 6;
		}
	}
	m_iBodyElems = ii/3;

	pBody = NULL;

	m_vboBody.destroy();
	m_vboBody.create();
	m_vboBody.bind();
	m_vboBody.allocate(bodyVertexArray, bodyVertexSize * sizeof(GLfloat));
	m_vboBody.release();

	delete [] bodyVertexArray;
}


void GL3Widget::initializeGL()
{
	glMakeUnitSphere();
	glMakeArcBall();
	glMakeArcPoint();

	//setup the shader to paint lines
	m_ShaderProgramLine.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/src/viewwidgets/shaders/line_vertexshader.glsl"));
#ifdef QT_DEBUG
	qDebug()<<m_ShaderProgramLine.log();
#endif
	m_ShaderProgramLine.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/src/viewwidgets/shaders/line_fragmentshader.glsl"));
#ifdef QT_DEBUG
	qDebug()<<m_ShaderProgramLine.log();
#endif
	m_ShaderProgramLine.link();
	m_ShaderProgramLine.bind();
	m_VertexLocationLine = m_ShaderProgramLine.attributeLocation("vertex");
	m_MatrixLocationLine = m_ShaderProgramLine.uniformLocation("pvmMatrix");
	m_ColorLocationLine  = m_ShaderProgramLine.uniformLocation("color");
	m_ShaderProgramLine.release();


	//setup the shader to paint the Cp and other gradients
	m_ShaderProgramGradient.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/src/viewwidgets/shaders/gradient_vertexshader.glsl"));
#ifdef QT_DEBUG
	qDebug()<<m_ShaderProgramGradient.log();
#endif
	m_ShaderProgramGradient.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/src/viewwidgets/shaders/gradient_fragmentshader.glsl"));
#ifdef QT_DEBUG
	qDebug()<<m_ShaderProgramGradient.log();
#endif
	m_ShaderProgramGradient.link();
	m_ShaderProgramGradient.bind();
	m_VertexLocationGradient = m_ShaderProgramGradient.attributeLocation("vertexPosition_modelspace");
	m_MatrixLocationGradient = m_ShaderProgramGradient.uniformLocation("pvmMatrix");
	m_ColorLocationGradient  = m_ShaderProgramGradient.attributeLocation("vertexColor");
	m_ShaderProgramGradient.release();




	//setup the shader to paint colored and textured surfaces
	m_ShaderProgramTexture.addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/src/viewwidgets/shaders/text_vertexshader.glsl"));
#ifdef QT_DEBUG
	qDebug()<<m_ShaderProgramTexture.log();
#endif
	m_ShaderProgramTexture.addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/src/viewwidgets/shaders/text_fragmentshader.glsl"));
#ifdef QT_DEBUG
	qDebug()<<m_ShaderProgramTexture.log();
#endif
	m_ShaderProgramTexture.link();

	m_ShaderProgramTexture.bind();

	m_VertexLocationTexture = m_ShaderProgramTexture.attributeLocation("vertexPosition_modelspace");
	m_NormalLocationTexture = m_ShaderProgramTexture.attributeLocation("vertexNormal_modelspace");
	m_UVLocationTexture     = m_ShaderProgramTexture.attributeLocation("vertexUV");

	m_pvmMatrixLocationTexture     = m_ShaderProgramTexture.uniformLocation("pvmMatrix");
	m_vMatrixLocationTexture       = m_ShaderProgramTexture.uniformLocation("vMatrix");
	m_mMatrixLocationTexture       = m_ShaderProgramTexture.uniformLocation("mMatrix");

	m_LightPosLocationTexture      = m_ShaderProgramTexture.uniformLocation("LightPosition_worldspace");
	m_LightColorLocationTexture    = m_ShaderProgramTexture.uniformLocation("LightColor");
	m_LightAmbientLocationTexture  = m_ShaderProgramTexture.uniformLocation("LightAmbient");
	m_LightDiffuseLocationTexture  = m_ShaderProgramTexture.uniformLocation("LightDiffuse");
	m_LightSpecularLocationTexture = m_ShaderProgramTexture.uniformLocation("LightSpecular");

	m_ColorLocationTexture         = m_ShaderProgramTexture.uniformLocation("incolor");//unused
	m_LightLocationTexture         = m_ShaderProgramTexture.uniformLocation("lightOn");
	m_TextureLocationTexture       = m_ShaderProgramTexture.uniformLocation("hasTexture");

	m_MaterialShininessTexture     = m_ShaderProgramTexture.uniformLocation("MaterialShininess");

	m_AttenuationConstantTexture   = m_ShaderProgramTexture.uniformLocation("Kc");
	m_AttenuationLinearTexture     = m_ShaderProgramTexture.uniformLocation("Kl");
	m_AttenuationQuadraticTexture  = m_ShaderProgramTexture.uniformLocation("Kq");

	m_ShaderProgramTexture.release();
	glSetupLight();
}


void GL3Widget::glSetupLight()
{
	QColor LightColor;
	LightColor.setRedF(  GLLightDlg::s_Light.m_Red);
	LightColor.setGreenF(GLLightDlg::s_Light.m_Green);
	LightColor.setBlueF( GLLightDlg::s_Light.m_Blue);

	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture,         GLLightDlg::s_Light.m_bIsLightOn);
	m_ShaderProgramTexture.setUniformValue(m_LightPosLocationTexture,      (GLfloat)(GLLightDlg::s_Light.m_X), (GLfloat)(GLLightDlg::s_Light.m_Y), (GLfloat)(GLLightDlg::s_Light.m_Z));
	m_ShaderProgramTexture.setUniformValue(m_LightColorLocationTexture,    LightColor);
	m_ShaderProgramTexture.setUniformValue(m_LightAmbientLocationTexture,  GLLightDlg::s_Light.m_Ambient);
	m_ShaderProgramTexture.setUniformValue(m_LightDiffuseLocationTexture,  GLLightDlg::s_Light.m_Diffuse);
	m_ShaderProgramTexture.setUniformValue(m_LightSpecularLocationTexture, GLLightDlg::s_Light.m_Specular);
	m_ShaderProgramTexture.setUniformValue(m_MaterialShininessTexture,     GLLightDlg::s_Material.m_iShininess);
	m_ShaderProgramTexture.setUniformValue(m_AttenuationConstantTexture,   GLLightDlg::s_Attenuation.m_Constant);
	m_ShaderProgramTexture.setUniformValue(m_AttenuationLinearTexture,     GLLightDlg::s_Attenuation.m_Linear);
	m_ShaderProgramTexture.setUniformValue(m_AttenuationQuadraticTexture,  GLLightDlg::s_Attenuation.m_Quadratic);
	m_ShaderProgramTexture.release();
}


void GL3Widget::paintOverlay()
{
	QOpenGLPaintDevice device(size() * devicePixelRatio());
	QPainter painter(&device);
	if(m_iView==XFLR5::GLMIAREXVIEW)
	{
		QMiarex* pMiarex = (QMiarex*)s_pMiarex;
		if(pMiarex->m_bResetTextLegend) pMiarex->drawTextLegend();

		painter.drawPixmap(0,0, pMiarex->m_PixText);
		painter.drawPixmap(0,0, m_PixTextOverlay);
		m_PixTextOverlay.fill(Qt::transparent);
	}
	else if(m_iView == XFLR5::GLBODYVIEW)
	{
//		GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
//		pDlg->paintBodyLegend(painter);
	}
	else if(m_iView == XFLR5::GLWINGVIEW)
	{
		painter.drawPixmap(0,0, m_PixTextOverlay);
		m_PixTextOverlay.fill(Qt::transparent);
	}
	else if(m_iView == XFLR5::GLPLANEVIEW)
	{
		EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
		painter.drawPixmap(0,0, pDlg->m_PixText);
		painter.drawPixmap(0,0, m_PixTextOverlay);
		m_PixTextOverlay.fill(Qt::transparent);
	}
	else if(m_iView == XFLR5::GLEDITBODYVIEW)
	{
		EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
		painter.drawPixmap(0,0, pDlg->m_PixText);
		painter.drawPixmap(0,0, m_PixTextOverlay);
		m_PixTextOverlay.fill(Qt::transparent);
	}
}

void GL3Widget::paintGL()
{
	switch(m_iView)
	{
		case XFLR5::GLMIAREXVIEW:
		{
			QMiarex *pMiarex = (QMiarex*)s_pMiarex;
			pMiarex->glMake3DObjects();
			break;
		}
		case XFLR5::GLWINGVIEW:
		{
			GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
			pDlg->glMake3DObjects();
			break;
		}
		case XFLR5::GLPLANEVIEW:
		{
			EditPlaneDlg *pDlg = (EditPlaneDlg*)m_pParent;
			pDlg->glMake3DObjects();
			break;
		}
		case XFLR5::GLBODYVIEW:
		{
			GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
			pDlg->glMake3DObjects();
			break;
		}
		case XFLR5::GLEDITBODYVIEW:
		{
			EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
			pDlg->glMake3DObjects();
			break;
		}
	}

	paintGL3();
	paintOverlay();
}


void GL3Widget::paintGL3()
{
	makeCurrent();

	int width, height;
	glClearColor(m_clearColor.redF(), m_clearColor.greenF(), m_clearColor.blueF(), m_clearColor.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	double s, pixelRatio;
	s = 1.0;
	pixelRatio = devicePixelRatio();

	width = geometry().width() * pixelRatio;
	height = geometry().height() * pixelRatio;

	m_OrthoMatrix.setToIdentity();
	m_OrthoMatrix.ortho(-s,s,-(height*s)/width,(height*s)/width,-100.0*s,100.0*s);

	QMatrix4x4 mat44(m_ArcBall.ab_quat);


	QMatrix4x4 modelMatrix;//keep identity
	m_viewMatrix.setToIdentity();
	m_viewMatrix *= mat44.transposed();
	m_pvmMatrix = m_OrthoMatrix * m_viewMatrix * modelMatrix;
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);
	if(m_bArcball) paintArcBall();

	m_viewMatrix.scale(m_glScaled, m_glScaled, m_glScaled);
	m_viewMatrix.translate(m_glRotCenter.x, m_glRotCenter.y, m_glRotCenter.z);
	m_pvmMatrix = m_OrthoMatrix * m_viewMatrix * modelMatrix;
	if(m_bAxes)    paintAxes();

	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.setUniformValue(m_mMatrixLocationTexture, modelMatrix);
	m_ShaderProgramTexture.setUniformValue(m_vMatrixLocationTexture, m_viewMatrix);
	m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);
	m_ShaderProgramTexture.release();


	switch(m_iView)
	{
		case XFLR5::GLMIAREXVIEW:
		{
			glRenderMiarexView();
			break;
		}
		case XFLR5::GLWINGVIEW:
		{
			glRenderWingView();
			break;
		}
		case XFLR5::GLPLANEVIEW:
		{
			glRenderPlaneView();
			break;
		}
		case XFLR5::GLBODYVIEW:
		{
			glRenderGL3DBodyView();
			break;
		}
		case XFLR5::GLEDITBODYVIEW:
		{
			glRenderEditBodyView();
			break;
		}
	}

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}


void GL3Widget::glRenderMiarexView()
{
	QMiarex* pMiarex = (QMiarex*)s_pMiarex;
	if(pMiarex->m_iView!=XFLR5::W3DVIEW) return;

	QMatrix4x4 modelMatrix;//keep identity
	if(pMiarex->m_pCurPOpp)
		modelMatrix.rotate(pMiarex->m_pCurPOpp->alpha(),0.0,1.0,0.0);
	m_pvmMatrix = m_OrthoMatrix * m_viewMatrix * modelMatrix;
	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.setUniformValue(m_mMatrixLocationTexture, modelMatrix);
	m_ShaderProgramTexture.setUniformValue(m_vMatrixLocationTexture, m_viewMatrix);
	m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);
	m_ShaderProgramTexture.release();

	if(pMiarex->m_pCurPlane)
	{
		paintBody(pMiarex->m_pCurPlane->body());
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			Wing * pWing = pMiarex->m_pCurPlane->wing(iw);
			if(pWing)
			{
				paintWing(iw, pWing);
				if(m_bFoilNames) glDrawFoils(pWing);
			}
		}
		if(m_bShowMasses) pMiarex->glDrawMasses();
		if(m_bVLMPanels)
		{
			paintMesh();
		}
		if(pMiarex->m_pCurPOpp)
		{
			if(pMiarex->m_b3DCp && pMiarex->m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
			{
				paintPanelCp();
			}

			if(pMiarex->m_bXCP)
			{
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintLift(iw);
				}
			}
			if(pMiarex->m_bDownwash)
			{
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintDownwash(iw);
				}
			}
			if(pMiarex->m_bICd)
			{
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintDrag(iw);
				}
			}
			if(pMiarex->m_bVCd)
			{
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintDrag(iw);
				}
			}
			if(pMiarex->m_bXTop || pMiarex->m_bXBot)
			{
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintTransitions(iw);
				}
			}

			modelMatrix.setToIdentity();//keep identity
			m_pvmMatrix = m_OrthoMatrix * m_viewMatrix * modelMatrix;
			m_ShaderProgramTexture.bind();
			m_ShaderProgramTexture.setUniformValue(m_mMatrixLocationTexture, modelMatrix);
			m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);

			if (pMiarex->m_bStream && pMiarex->m_pCurPOpp &&
				pMiarex->m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD && !pMiarex->m_bResetglStream)
				paintStreamLines();

			if (pMiarex->m_b3DCp && pMiarex->m_pCurPOpp && pMiarex->m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
			{
				paintCpLegendClr();
			}
			else if (pMiarex->m_bPanelForce && pMiarex->m_pCurPOpp && pMiarex->m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
			{
				paintCpLegendClr();
			}
		}
	}
}


void GL3Widget::glRenderPlaneView()
{
	QMiarex* pMiarex = (QMiarex*)s_pMiarex;
	if(pMiarex->m_iView!=XFLR5::W3DVIEW) return;

	if(pMiarex->m_pCurPlane)
	{
		paintBody(pMiarex->m_pCurPlane->body());
		for(int iw=0; iw<MAXWINGS; iw++)
		{
			Wing * pWing = pMiarex->m_pCurPlane->wing(iw);
			if(pWing)
			{
				paintWing(iw, pWing);
				if(m_bFoilNames) glDrawFoils(pWing);
			}
		}
		if(m_bShowMasses) pMiarex->glDrawMasses();
	}
}

void GL3Widget::glRenderGL3DBodyView()
{
	GL3dBodyDlg *pDlg = (GL3dBodyDlg*)m_pParent;
	Body *pBody = pDlg->m_pBody;

	if(pBody)
	{
		paintBody(pDlg->m_pBody);
		if(pDlg->m_pFrame) paintSectionHighlight();
	}

	if(m_bShowMasses) glDrawMasses(pBody->volumeMass(), pDlg->m_pBody->CoG(), "Structural mass", pBody->m_PointMass);
}



void GL3Widget::glRenderEditBodyView()
{
	EditBodyDlg *pDlg = (EditBodyDlg*)m_pParent;
	Body *pBody = pDlg->m_pBody;

	if(pBody)
	{
		paintBody(pDlg->m_pBody);
	}

	if(m_bShowMasses) glDrawMasses(pBody->volumeMass(), pDlg->m_pBody->CoG(), "Structural mass", pBody->m_PointMass );
}



void GL3Widget::glRenderWingView()
{
	GL3dWingDlg *pDlg = (GL3dWingDlg*)m_pParent;
	Wing *pWing = pDlg->m_pWing;

	if(pWing)
	{
		paintWing(0, pWing);
		if(m_bFoilNames) glDrawFoils(pWing);
		if(m_bVLMPanels) paintWingMesh(pWing);
		if(m_bShowMasses) glDrawMasses(pWing->volumeMass(), pWing->CoG(), "Structural mass", pWing->m_PointMass);
		if(pDlg->m_iSection>=0) paintSectionHighlight();
	}
}





void GL3Widget::setScale(double refLength)
{
	m_glScaled = 1.5/refLength;
}




void GL3Widget::glDrawFoils(void *pWingPtr)
{
	int j;
	Foil *pFoil;
	Wing *pWing = (Wing*)pWingPtr;

	for(j=0; j<pWing->m_Surface.size(); j++)
	{
		pFoil = pWing->m_Surface.at(j)->foilA();

		if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TA.x, pWing->m_Surface.at(j)->m_TA.y, pWing->m_Surface.at(j)->m_TA.z,
							   pFoil->foilName(),
							   QColor(Qt::cyan).lighter(175));

	}

	j = pWing->m_Surface.size()-1;
	pFoil = pWing->m_Surface.at(j)->foilB();
	if(pFoil) glRenderText(pWing->m_Surface.at(j)->m_TB.x, pWing->m_Surface.at(j)->m_TB.y, pWing->m_Surface.at(j)->m_TB.z,
						 pFoil->foilName(),
						 QColor(Qt::cyan).lighter(175));
}



void GL3Widget::glDrawMasses(double volumeMass, CVector pos, QString tag, QList<PointMass*> ptMasses)
{
	if(qAbs(volumeMass)>PRECISION)
	{
		glRenderText(pos.x, pos.y, pos.z,
					 tag + tag + QString(" (%1").arg(volumeMass*Units::kgtoUnit(), 0,'g',3) + Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
	}

	for(int im=0; im<ptMasses.size(); im++)
	{
		paintSphere(ptMasses[im]->position(), W3dPrefsDlg::s_MassRadius/m_glScaled, W3dPrefsDlg::s_MassColor, false);
		glRenderText(ptMasses[im]->position().x,
					 ptMasses[im]->position().y,
					 ptMasses[im]->position().z +.02/m_glScaled,
					 ptMasses[im]->tag()+QString(" (%1").arg(ptMasses[im]->mass()*Units::kgtoUnit(), 0,'g',3)+Units::weightUnitLabel()+")", W3dPrefsDlg::s_MassColor.lighter(125));
	}
}




void GL3Widget::paintPanelCp()
{
	m_ShaderProgramGradient.bind();
	m_ShaderProgramGradient.enableAttributeArray(m_VertexLocationGradient);
	m_ShaderProgramGradient.enableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.setUniformValue(m_MatrixLocationGradient, m_pvmMatrix);
	m_vboPanelCp.bind();
	m_ShaderProgramGradient.setAttributeBuffer(m_VertexLocationGradient, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
	m_ShaderProgramGradient.setAttributeBuffer(m_ColorLocationGradient,  GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glDrawArrays(GL_TRIANGLES, 0, Objects3D::s_MatSize*2*3);
	glDisable(GL_POLYGON_OFFSET_FILL);

	m_ShaderProgramGradient.disableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.disableAttributeArray(m_VertexLocationGradient);
	m_vboPanelCp.release();
	m_ShaderProgramGradient.release();
}


void GL3Widget::paintCpLegendClr()
{
	m_ShaderProgramGradient.bind();
	m_ShaderProgramGradient.enableAttributeArray(m_VertexLocationGradient);
	m_ShaderProgramGradient.enableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.setUniformValue(m_MatrixLocationGradient, m_OrthoMatrix);
	m_vboLegendColor.bind();
	m_ShaderProgramGradient.setAttributeBuffer(m_VertexLocationGradient, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
	m_ShaderProgramGradient.setAttributeBuffer(m_ColorLocationGradient,  GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, MAXCPCOLORS*2);
	glDisable(GL_POLYGON_OFFSET_FILL);

	m_ShaderProgramGradient.disableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.disableAttributeArray(m_VertexLocationGradient);
	m_vboLegendColor.release();
	m_ShaderProgramGradient.release();
}


void GL3Widget::paintMesh()
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_vboMesh.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);

	int pos = 0;
	for(int p=0; p<Objects3D::s_MatSize*2; p++)
	{
		glDrawArrays(GL_LINE_STRIP, pos, 3);
		pos +=3 ;
	}

	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, Settings::s_BackgroundColor);

	QMiarex *pMiarex = (QMiarex*)s_pMiarex;
	if(!pMiarex->m_pCurPOpp || !pMiarex->m_b3DCp)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		glDrawArrays(GL_TRIANGLES, 0, Objects3D::s_MatSize*2*3);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_vboMesh.release();
	m_ShaderProgramLine.release();
}



/** used only in GL3DWingDlg*/
void GL3Widget::paintSectionHighlight()
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_vboHighlight.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(255,0,0));
	glLineWidth(5);

	int pos = 0;
	for(int iLines=0; iLines<m_nHighlightLines; iLines++)
	{
		glDrawArrays(GL_LINE_STRIP, pos, m_HighlightLineSize);
		pos += m_HighlightLineSize;
	}

	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_vboHighlight.release();
	m_ShaderProgramLine.release();
}


/** Used only in GL3DWingDlg, at a time when the mesh panels have not yet been built */
void GL3Widget::paintWingMesh(Wing *pWing)
{
	if(!pWing) return;

	int pos;

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_vboWingMesh.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VLMColor);

	glEnable (GL_LINE_STIPPLE);
	switch(W3dPrefsDlg::s_VLMStyle)
	{
		case 1:  glLineStipple (1, 0xCFCF); break;
		case 2:  glLineStipple (1, 0x6666); break;
		case 3:  glLineStipple (1, 0xFF18); break;
		case 4:  glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}
	glLineWidth(W3dPrefsDlg::s_VLMWidth);


	pos=0;
	for(int j=0; j<pWing->m_Surface.count(); j++)
	{
		//top chordwise lines
		for(int k=0; k<pWing->m_Surface.at(j)->m_NYPanels+1; k++)
		{
			glDrawArrays(GL_LINE_STRIP, pos, pWing->m_Surface.at(j)->m_NXPanels+1);
			pos += pWing->m_Surface.at(j)->m_NXPanels+1;
		}
		//bot chordwise lines
		for(int k=0; k<pWing->m_Surface.at(j)->m_NYPanels+1; k++)
		{
			glDrawArrays(GL_LINE_STRIP, pos, pWing->m_Surface.at(j)->m_NXPanels+1);
			pos += pWing->m_Surface.at(j)->m_NXPanels+1;
		}
		//top spanwise lines
		for(int l=0; l<pWing->m_Surface.at(j)->m_NXPanels+1; l++)
		{
			glDrawArrays(GL_LINE_STRIP, pos, pWing->m_Surface.at(j)->m_NYPanels+1);
			pos += pWing->m_Surface.at(j)->m_NYPanels+1;
		}
		//bot spanwise lines
		for(int l=0; l<pWing->m_Surface.at(j)->m_NXPanels+1; l++)
		{
			glDrawArrays(GL_LINE_STRIP, pos, pWing->m_Surface.at(j)->m_NYPanels+1);
			pos += pWing->m_Surface.at(j)->m_NYPanels+1;
		}
	}
	//tip patches
	pos +=1;
	int tipPos =pos;
	for(int j=0; j<pWing->m_Surface.count(); j++)
	{
		if(pWing->m_Surface.at(j)->isTipLeft())
		{
			for(int l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				glDrawArrays(GL_LINES, pos, 2);
				pos += 2;
			}
		}
		if(pWing->m_Surface.at(j)->isTipRight())
		{
			for(int l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				glDrawArrays(GL_LINES, pos, 2);
				pos += 2;
			}
		}
	}

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	pos=tipPos;
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, Settings::s_BackgroundColor);
	for(int j=0; j<pWing->m_Surface.count(); j++)
	{
		if(pWing->m_Surface.at(j)->isTipLeft())
		{
			glDrawArrays(GL_TRIANGLE_STRIP, pos, (pWing->m_Surface.at(j)->m_NXPanels+1)*2);
			pos += (pWing->m_Surface.at(j)->m_NXPanels+1)*2;
		}
		if(pWing->m_Surface.at(j)->isTipRight())
		{
			glDrawArrays(GL_TRIANGLE_STRIP, pos, (pWing->m_Surface.at(j)->m_NXPanels+1)*2);
			pos += (pWing->m_Surface.at(j)->m_NXPanels+1)*2;
		}
	}


	for(int j=0; j<pWing->m_Surface.count(); j++)
	{
		for(int k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, pos, (pWing->m_Surface.at(j)->m_NXPanels+1)*2);
			pos += (pWing->m_Surface.at(j)->m_NXPanels+1)*2;
			glDrawArrays(GL_TRIANGLE_STRIP, pos, (pWing->m_Surface.at(j)->m_NXPanels+1)*2);
			pos += (pWing->m_Surface.at(j)->m_NXPanels+1)*2;
		}
	}

	m_vboWingMesh.release();
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);


}


void GL3Widget::paintArcBall()
{
	int pos;
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_vboArcBall.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(50,55,80,255));

	pos=0;
	for (int col=0; col<NUMCIRCLES*2; col++)
	{
		glDrawArrays(GL_LINE_STRIP, pos, NUMANGLES-2);
		pos += NUMANGLES-2;
	}
	glDrawArrays(GL_LINE_STRIP, pos, NUMPERIM-1);
	pos += NUMPERIM-1;
	glDrawArrays(GL_LINE_STRIP, pos, NUMPERIM-1);
	pos += NUMPERIM-1;
	m_vboArcBall.release();

	if(m_bCrossPoint)
	{
		double s, pixelRatio;
		s = 1.0;
		pixelRatio = devicePixelRatio();

		int width = geometry().width() * pixelRatio;
		int height = geometry().height() * pixelRatio;

		QMatrix4x4 pvmCP; /** @todo remove pvm as parameter */
		pvmCP.setToIdentity();
		pvmCP.ortho(-s,s,-(height*s)/width,(height*s)/width,-100.0*s,100.0*s);

		m_ArcBall.rotateCrossPoint();
		pvmCP.rotate(m_ArcBall.angle, m_ArcBall.p.x, m_ArcBall.p.y, m_ArcBall.p.z);
		m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, pvmCP);

		m_vboArcPoint.bind();
		m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, QColor(70, 25, 40));
		pos=0;
		glLineWidth(3.0);
		glDrawArrays(GL_LINE_STRIP, pos, 2*NUMARCPOINTS);
		pos += 2*NUMARCPOINTS;
		glDrawArrays(GL_LINE_STRIP, pos, 2*NUMARCPOINTS);
		glLineWidth(1.0);
		m_vboArcPoint.release();
	}
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}


void GL3Widget::paintAxes()
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 0);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_3DAxisColor);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	//draw Axis
	glLineWidth(W3dPrefsDlg::s_3DAxisWidth);
	glEnable (GL_LINE_STIPPLE);
	glLineStipple (1, 0xFF18);

	m_ShaderProgramLine.setAttributeArray(m_VertexLocationLine, x_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);
	m_ShaderProgramLine.setAttributeArray(m_VertexLocationLine, y_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);
	m_ShaderProgramLine.setAttributeArray(m_VertexLocationLine, z_axis, 3);
	glDrawArrays(GL_LINE_STRIP, 0, 6);

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
	glRenderText(1.0, 0.015, 0.015, "X");
	glRenderText(0.015, 1.0, 0.015, "Y");
	glRenderText(0.015, 0.015, 1.0, "Z");
}


void GL3Widget::paintDrag(int iWing)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	glEnable(GL_DEPTH_TEST);
	glEnable (GL_LINE_STIPPLE);

	// Induced drag
	m_vboICd[iWing].bind();
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_IDragColor);
	switch(W3dPrefsDlg::s_IDragStyle)
	{
		case 1:  glLineStipple (1, 0xCFCF); break;
		case 2:  glLineStipple (1, 0x6666); break;
		case 3:  glLineStipple (1, 0xFF18); break;
		case 4:  glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}
	glLineWidth((GLfloat)W3dPrefsDlg::s_IDragWidth);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);

//	for(int iy=0; iy<m_Ny[iWing]; iy++)		glDrawArrays(GL_LINES, 2*iy, 2);

	glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
//	glDrawArrays(GL_LINE_STRIP, 2*m_Ny[iWing], m_Ny[iWing]);
	glDrawArrays(GL_LINE_STRIP, m_Ny[iWing]*2, m_Ny[iWing]);

	m_vboICd[iWing].release();



	//Viscous drag
	m_vboVCd[iWing].bind();
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_VDragColor);
	switch(W3dPrefsDlg::s_VDragStyle)
	{
		case 1: glLineStipple (1, 0xCFCF); break;
		case 2: glLineStipple (1, 0x6666); break;
		case 3: glLineStipple (1, 0xFF18); break;
		case 4: glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}
	glLineWidth((GLfloat)W3dPrefsDlg::s_VDragWidth);

	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);

	glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
	glDrawArrays(GL_LINE_STRIP, 2*m_Ny[iWing], m_Ny[iWing]);
	m_vboVCd[iWing].release();


	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}




void GL3Widget::paintStreamLines()
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	m_vboStreamLines.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	glEnable(GL_DEPTH_TEST);
	glEnable (GL_LINE_STIPPLE);

	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_StreamLinesColor);
	switch(W3dPrefsDlg::s_StreamLinesStyle)
	{
		case 1: glLineStipple (1, 0xCFCF); break;
		case 2: glLineStipple (1, 0x6666); break;
		case 3: glLineStipple (1, 0xFF18); break;
		case 4: glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}
	glLineWidth((GLfloat)W3dPrefsDlg::s_StreamLinesWidth);

	int pos=0;

	for(int il=0; il<m_NStreamLines; il++)
	{
		glDrawArrays(GL_LINE_STRIP, pos, GL3DScales::s_NX);
		pos += GL3DScales::s_NX;
	}

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_vboStreamLines.release();
	m_ShaderProgramLine.release();
}


void GL3Widget::paintTransitions(int iWing)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	m_vboTransitions[iWing].bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);

	glEnable(GL_DEPTH_TEST);
	glEnable (GL_LINE_STIPPLE);

	QMiarex *pMiarex= (QMiarex*)s_pMiarex;
	if(pMiarex->m_bXTop)
	{
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_TopColor);
		switch(W3dPrefsDlg::s_TopStyle)
		{
			case 1: glLineStipple (1, 0xCFCF); break;
			case 2: glLineStipple (1, 0x6666); break;
			case 3: glLineStipple (1, 0xFF18); break;
			case 4: glLineStipple (1, 0x7E66); break;
			default: glLineStipple (1, 0xFFFF); break;
		}
		glLineWidth((GLfloat)W3dPrefsDlg::s_TopWidth);
		glDrawArrays(GL_LINE_STRIP, 0, m_Ny[iWing]);
	}


	if(pMiarex->m_bXBot)
	{
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_BotColor);
		switch(W3dPrefsDlg::s_BotStyle)
		{
			case 1: glLineStipple (1, 0xCFCF); break;
			case 2: glLineStipple (1, 0x6666); break;
			case 3: glLineStipple (1, 0xFF18); break;
			case 4: glLineStipple (1, 0x7E66); break;
			default: glLineStipple (1, 0xFFFF); break;
		}
		glLineWidth((GLfloat)W3dPrefsDlg::s_BotWidth);
		glDrawArrays(GL_LINE_STRIP, m_Ny[iWing], m_Ny[iWing]);
	}


	m_vboTransitions[iWing].release();

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}



void GL3Widget::glMakeDragStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
{
	if(!pWing || !pWPolar || !pWOpp) return;
	CVector C, Pt, PtNormal;
	int i,j,k;
	float *pICdVertexArray, *pVCdVertexArray;
	QMiarex *pMiarex = (QMiarex*)s_pMiarex;

	double coef = 5.0;
	double amp, amp1, amp2, yob, dih, cosa, cosb, sina, sinb;
	cosa =  cos(pWOpp->m_Alpha * PI/180.0);
	sina = -sin(pWOpp->m_Alpha * PI/180.0);
	cosb =  cos(pWPolar->sideSlip()*PI/180.0);
	sinb =  sin(pWPolar->sideSlip()*PI/180.0);

	pICdVertexArray = new float[m_Ny[iWing]*9];
	pVCdVertexArray = new float[m_Ny[iWing]*9];

	//DRAGLINE
	double q0 = 0.5 * pWPolar->density() * pWPolar->referenceArea() * pWOpp->m_QInf * pWOpp->m_QInf;

	if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
		int ii=0;
		int iv=0;
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;

			pWing->surfacePoint(1.0, yob, MIDSURFACE, Pt, PtNormal);

			dih = pWing->Dihedral(yob)*PI/180.0;
			amp1 = q0*pWOpp->m_ICd[i]*pWing->getChord(yob)/pWOpp->m_MAChord*QMiarex::s_DragScale/coef;
			amp2 = q0*pWOpp->m_PCd[i]*pWing->getChord(yob)/pWOpp->m_MAChord*QMiarex::s_DragScale/coef;
			if(pMiarex->m_bICd)
			{
				pICdVertexArray[ii++] = Pt.x;
				pICdVertexArray[ii++] = Pt.y;
				pICdVertexArray[ii++] = Pt.z;
				pICdVertexArray[ii++] = Pt.x + amp1 * cos(dih)*cosa;
				pICdVertexArray[ii++] = Pt.y;
				pICdVertexArray[ii++] = Pt.z - amp1 * cos(dih)*sina;
			}
			if(pMiarex->m_bVCd)
			{
				if(!pMiarex->m_bICd)
				{
					pVCdVertexArray[iv++] = Pt.x;
					pVCdVertexArray[iv++] = Pt.y;
					pVCdVertexArray[iv++] = Pt.z;
					pVCdVertexArray[iv++] = Pt.x + amp2 * cos(dih)*cosa;
					pVCdVertexArray[iv++] = Pt.y;
					pVCdVertexArray[iv++] = Pt.z - amp2 * cos(dih)*sina;
				}
				else
				{
					pVCdVertexArray[iv++] = Pt.x + amp1 * cos(dih)*cosa;
					pVCdVertexArray[iv++] = Pt.y;
					pVCdVertexArray[iv++] = Pt.z - amp1 * cos(dih)*sina;

					pVCdVertexArray[iv++] = Pt.x + (amp1+amp2) * cos(dih)*cosa;
					pVCdVertexArray[iv++] = Pt.y;
					pVCdVertexArray[iv++] = Pt.z - (amp1+amp2) * cos(dih)*sina;
				}
			}
		}
		if(pMiarex->m_bICd)
		{
			for (i=1; i<pWOpp->m_NStation; i++)
			{
				yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
				pWing->surfacePoint(1.0, yob, MIDSURFACE, Pt, PtNormal);

				dih = pWing->Dihedral(yob)*PI/180.0;
				amp  = q0*pWOpp->m_ICd[i]*pWing->getChord(yob)/pWOpp->m_MAChord;
				amp *= QMiarex::s_DragScale/coef;

				pICdVertexArray[ii++] = Pt.x + amp * cos(dih)*cosa;
				pICdVertexArray[ii++] = Pt.y;
				pICdVertexArray[ii++] = Pt.z - amp * cos(dih)*sina;
			}
		}
		if(pMiarex->m_bVCd)
		{
			for (i=1; i<pWOpp->m_NStation; i++)
			{
				yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
				pWing->surfacePoint(1.0, yob, MIDSURFACE, Pt, PtNormal);

				dih = pWing->Dihedral(yob)*PI/180.0;
				amp=0.0;
				if(pMiarex->m_bICd) amp+=pWOpp->m_ICd[i];
				amp +=pWOpp->m_PCd[i];
				amp *= q0*pWing->getChord(yob)/pWOpp->m_MAChord;
				amp *= QMiarex::s_DragScale/coef;

				pVCdVertexArray[iv++] = Pt.x + amp * cos(dih)*cosa;
				pVCdVertexArray[iv++] = Pt.y;
				pVCdVertexArray[iv++] = Pt.z - amp * cos(dih)*sina;
			}
		}
	}
	else
	{
		//Panel type drag
		i = 0;
		int ii=0;
		int iv=0;
		for (j=0; j<pWing->m_Surface.size(); j++)
		{
			//All surfaces
			for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
			{
				pWing->m_Surface[j]->getTrailingPt(k, C);
				amp1 = q0*pWOpp->m_ICd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord*QMiarex::s_DragScale/coef;
				amp2 = q0*pWOpp->m_PCd[i]*pWOpp->m_Chord[i]/pWing->m_MAChord*QMiarex::s_DragScale/coef;
				if(pMiarex->m_bICd)
				{
					pICdVertexArray[ii++] = C.x;
					pICdVertexArray[ii++] = C.y;
					pICdVertexArray[ii++] = C.z;
					pICdVertexArray[ii++] = C.x + amp1*cosa * cosb;
					pICdVertexArray[ii++] = C.y + amp1*cosa * sinb;
					pICdVertexArray[ii++] = C.z - amp1*sina;
				}
				if(pMiarex->m_bVCd)
				{
					if(!pMiarex->m_bICd)
					{
						pVCdVertexArray[iv++] = C.x;
						pVCdVertexArray[iv++] = C.y;
						pVCdVertexArray[iv++] = C.z;
						pVCdVertexArray[iv++] = C.x + amp2*cosa * cosb;
						pVCdVertexArray[iv++] = C.y + amp2*cosa * sinb;
						pVCdVertexArray[iv++] = C.z - amp2*sina;
					}
					else
					{
						pVCdVertexArray[iv++] = C.x + amp1*cosa*cosb;
						pVCdVertexArray[iv++] = C.y + amp1*cosa*sinb;
						pVCdVertexArray[iv++] = C.z - amp1*sina;
						pVCdVertexArray[iv++] = C.x + (amp1+amp2)*cosa*cosb;
						pVCdVertexArray[iv++] = C.y + (amp1+amp2)*cosa*sinb;
						pVCdVertexArray[iv++] = C.z - (amp1+amp2)*sina;
					}
				}

				i++;
			}
		}
		if(!pWing->isFin())
		{
			if(pMiarex->m_bICd)
			{
				i = 0;
				for (j=0; j<pWing->m_Surface.size(); j++)
				{
					for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
					{
						pWing->m_Surface[j]->getTrailingPt(k, C);
						amp = q0*(pWOpp->m_ICd[i]*pWOpp->m_Chord[i])/pWing->m_MAChord;
						amp *= QMiarex::s_DragScale/coef;
						pICdVertexArray[ii++] = C.x + amp*cosa * cosb;
						pICdVertexArray[ii++] = C.y + amp*cosa * sinb;
						pICdVertexArray[ii++] = C.z - amp*sina;
						i++;
					}
				}
			}
			if(pMiarex->m_bVCd)
			{
				i = 0;
				for (j=0; j<pWing->m_Surface.size(); j++)
				{
					for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
					{
						pWing->m_Surface[j]->getTrailingPt(k, C);
						amp=0.0;
						if(pMiarex->m_bICd) amp+=pWOpp->m_ICd[i];
						amp +=pWOpp->m_PCd[i];
						amp *= q0*pWOpp->m_Chord[i]/pWing->m_MAChord;
						amp *= QMiarex::s_DragScale/coef;

						pVCdVertexArray[iv++] = C.x + amp*cosa*cosb;
						pVCdVertexArray[iv++] = C.y + amp*cosa*sinb;
						pVCdVertexArray[iv++] = C.z - amp*sina;

						i++;
					}
				}
			}
		}
		else
		{
			if(pMiarex->m_bICd)
			{
				i = 0;
				for (j=0; j<pWing->m_Surface.size(); j++)
				{
					for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
					{
						pWing->m_Surface[j]->getTrailingPt(k, C);
						amp = q0*(pWOpp->m_ICd[i]*pWOpp->m_Chord[i])/pWing->m_MAChord;
						amp *= QMiarex::s_DragScale/coef;
						pICdVertexArray[ii++] = C.x + amp*cosa * cosb;
						pICdVertexArray[ii++] = C.y + amp*cosa * sinb;
						pICdVertexArray[ii++] = C.z - amp*sina;
						i++;
					}
				}
			}
			if(pMiarex->m_bVCd)
			{
				i = 0;
				for (j=0; j<pWing->m_Surface.size(); j++)
				{
					for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
					{
						pWing->m_Surface[j]->getTrailingPt(k, C);
						amp=0.0;
						if(pMiarex->m_bICd) amp+=pWOpp->m_ICd[i];
						amp +=pWOpp->m_PCd[i];
						amp *= q0*pWOpp->m_Chord[i]/pWing->m_MAChord;
						amp *= QMiarex::s_DragScale/coef;

						pVCdVertexArray[iv++] = C.x + amp*cosa*cosb;
						pVCdVertexArray[iv++] = C.y + amp*cosa*sinb;
						pVCdVertexArray[iv++] = C.z - amp*sina;
						i++;
					}
				}
			}
		}
	}

	m_vboICd[iWing].destroy();
	m_vboICd[iWing].create();
	m_vboICd[iWing].bind();
	m_vboICd[iWing].allocate(pICdVertexArray, m_Ny[iWing] *9 * sizeof(GLfloat));
	m_vboICd[iWing].release();
	delete [] pICdVertexArray;

	m_vboVCd[iWing].destroy();
	m_vboVCd[iWing].create();
	m_vboVCd[iWing].bind();
	m_vboVCd[iWing].allocate(pVCdVertexArray, m_Ny[iWing] *9 * sizeof(GLfloat));
	m_vboVCd[iWing].release();
	delete [] pVCdVertexArray;
}

void GL3Widget::setSpanStations(Plane *pPlane, WPolar *pWPolar, PlaneOpp *pPOpp)
{
	if(!pPlane || !pWPolar || !pPOpp) return;
	Wing *pWing;

	if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
		if(pPOpp)
		{
			m_Ny[0] = pPOpp->m_pPlaneWOpp[0]->m_NStation-1;
		}
		else
		{
			m_Ny[0] = LLTAnalysis::s_NLLTStations;
		}

		m_Ny[1] = m_Ny[2] = m_Ny[3] = 0;
	}
	else
	{
		for(int iWing=0; iWing<MAXWINGS; iWing++)
		{
			pWing = pPlane->wing(iWing);
			if(pWing)
			{
				m_Ny[iWing]=0;
				for (int j=0; j<pWing->m_Surface.size(); j++)
				{
					m_Ny[iWing] += pWing->m_Surface[j]->m_NYPanels;
				}
			}
		}
	}
}


void GL3Widget::glMakeDownwash(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
{
	if(!pWing || !pWPolar || !pWOpp) return;

	int i,j,k,p, iv;
	double dih, yob;
	double y1, y2, z1, z2, xs, ys, zs;
	CVector C, Pt, PtNormal;
	double factor, amp;

	double sina = -sin(pWOpp->m_Alpha*PI/180.0);
	double cosa =  cos(pWOpp->m_Alpha*PI/180.0);
	factor = QMiarex::s_VelocityScale/5.0;

	float *pDownWashVertexArray = new float[m_Ny[iWing]*18];

	if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
			pWing->surfacePoint(1.0, yob, MIDSURFACE, Pt, PtNormal);

			dih = -pWing->Dihedral(yob)*PI/180.0;
			amp = pWOpp->m_QInf*sin(pWOpp->m_Ai[i]*PI/180.0);
			amp *= factor;
			pDownWashVertexArray[iv++] = Pt.x;
			pDownWashVertexArray[iv++] = Pt.y;
			pDownWashVertexArray[iv++] = Pt.z;

			pDownWashVertexArray[iv++] = Pt.x + amp * cos(dih)* sina;
			pDownWashVertexArray[iv++] = Pt.y + amp * sin(dih);
			pDownWashVertexArray[iv++] = Pt.z + amp * cos(dih)* cosa;

			xs = Pt.x + amp * cos(dih) * sina;
			ys = Pt.y + amp * sin(dih);
			zs = Pt.z + amp * cos(dih) * cosa;
			y1 = ys - 0.085*amp * sin(dih)        + 0.05*amp * cos(dih) * cosa;
			z1 = zs - 0.085*amp * cos(dih) * cosa - 0.05*amp * sin(dih);
			y2 = ys - 0.085*amp * sin(dih)        - 0.05*amp * cos(dih) * cosa;
			z2 = zs - 0.085*amp * cos(dih) * cosa + 0.05*amp * sin(dih);

			pDownWashVertexArray[iv++] = xs;
			pDownWashVertexArray[iv++] = ys;
			pDownWashVertexArray[iv++] = zs;
			pDownWashVertexArray[iv++] = xs;
			pDownWashVertexArray[iv++] = y1;
			pDownWashVertexArray[iv++] = z1;

			pDownWashVertexArray[iv++] = xs;
			pDownWashVertexArray[iv++] = ys;
			pDownWashVertexArray[iv++] = zs;
			pDownWashVertexArray[iv++] = xs;
			pDownWashVertexArray[iv++] = y2;
			pDownWashVertexArray[iv++] = z2;
		}
	}
	else
	{
		p = 0;
		i = 0;
		iv = 0;
		for (j=0; j<pWing->m_Surface.size(); j++)
		{
			for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
			{
//						m_pSurface[j+surf0]->GetTrailingPt(k, C);
				pWing->m_Surface[j]->getTrailingPt(k, C);
//						if (pWOpp->m_Vd[i].z>0) sign = 1.0; else sign = -1.0;
				pDownWashVertexArray[iv++] = C.x;
				pDownWashVertexArray[iv++] = C.y;
				pDownWashVertexArray[iv++] = C.z;
				pDownWashVertexArray[iv++] = C.x+factor*pWOpp->m_Vd[i].z * sina;
				pDownWashVertexArray[iv++] = C.y+factor*pWOpp->m_Vd[i].y;
				pDownWashVertexArray[iv++] = C.z+factor*pWOpp->m_Vd[i].z * cosa;

				xs = C.x+factor*pWOpp->m_Vd[i].z*sina;
				ys = C.y+factor*pWOpp->m_Vd[i].y;
				zs = C.z+factor*pWOpp->m_Vd[i].z*cosa;
				y1 = ys - 0.085*factor*pWOpp->m_Vd[i].y      + 0.05*factor*pWOpp->m_Vd[i].z*cosa;
				z1 = zs - 0.085*factor*pWOpp->m_Vd[i].z*cosa - 0.05*factor*pWOpp->m_Vd[i].y;
				y2 = ys - 0.085*factor*pWOpp->m_Vd[i].y      - 0.05*factor*pWOpp->m_Vd[i].z*cosa;
				z2 = zs - 0.085*factor*pWOpp->m_Vd[i].z*cosa + 0.05*factor*pWOpp->m_Vd[i].y;

				pDownWashVertexArray[iv++] = xs;
				pDownWashVertexArray[iv++] = ys;
				pDownWashVertexArray[iv++] = zs;
				pDownWashVertexArray[iv++] = xs;
				pDownWashVertexArray[iv++] = y1;
				pDownWashVertexArray[iv++] = z1;

				pDownWashVertexArray[iv++] = xs;
				pDownWashVertexArray[iv++] = ys;
				pDownWashVertexArray[iv++] = zs;
				pDownWashVertexArray[iv++] = xs;
				pDownWashVertexArray[iv++] = y2;
				pDownWashVertexArray[iv++] = z2;

				i++;
			}
			p++;
		}
	}

	m_vboDownwash[iWing].destroy();
	m_vboDownwash[iWing].create();
	m_vboDownwash[iWing].bind();
	m_vboDownwash[iWing].allocate(pDownWashVertexArray, m_Ny[iWing]*18 * sizeof(GLfloat));
	m_vboDownwash[iWing].release();
	delete [] pDownWashVertexArray;
}


void GL3Widget::paintDownwash(int iWing)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_DownwashColor);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	m_vboDownwash[iWing].bind();
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_LINE_STIPPLE);

	switch(W3dPrefsDlg::s_DownwashStyle)
	{
		case 1: glLineStipple (1, 0xCFCF); break;
		case 2: glLineStipple (1, 0x6666); break;
		case 3: glLineStipple (1, 0xFF18); break;
		case 4: glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}

	glLineWidth((GLfloat)W3dPrefsDlg::s_DownwashWidth);

	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);

	glDrawArrays(GL_LINES, 0, m_Ny[iWing]*6);
	m_vboDownwash[iWing].release();

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}



void GL3Widget::glMakeLiftStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
{
	if(!pWing || !pWPolar || !pWOpp) return;
	int i,j,k;
	CVector C, CL, Pt, PtNormal;

	double amp, yob, dih;
	double cosa =  cos(pWOpp->m_Alpha * PI/180.0);
	double sina = -sin(pWOpp->m_Alpha * PI/180.0);

	//LIFTLINE
	//dynamic pressure x area
	double q0 = 0.5 * pWPolar->density() * pWOpp->m_QInf * pWOpp->m_QInf;
	float *pLiftVertexArray = new float[m_Ny[iWing]*9];

	if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{

		int iv=0;

		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
			pWing->surfacePoint(pWOpp->m_XCPSpanRel[i], yob, MIDSURFACE, Pt, PtNormal);
			dih = -pWing->Dihedral(yob)*PI/180.0;
			amp = q0*pWOpp->m_Cl[i]*pWing->getChord(yob)/pWOpp->m_MAChord;
			amp *= QMiarex::s_LiftScale/1000.0;

			pLiftVertexArray[iv++] = Pt.x;
			pLiftVertexArray[iv++] = Pt.y;
			pLiftVertexArray[iv++] = Pt.z;
			pLiftVertexArray[iv++] = Pt.x + amp * cos(dih)*sina;
			pLiftVertexArray[iv++] = Pt.y + amp * sin(dih);
			pLiftVertexArray[iv++] = Pt.z + amp * cos(dih)*cosa;
		}

		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
			pWing->surfacePoint(pWOpp->m_XCPSpanRel[i], yob, MIDSURFACE, Pt, PtNormal);

			dih = -pWing->Dihedral(yob)*PI/180.0;
			amp = q0*pWOpp->m_Cl[i]*pWing->getChord(yob)/pWOpp->m_MAChord;
			amp *= QMiarex::s_LiftScale/1000.0;

			pLiftVertexArray[iv++] = Pt.x + amp * cos(dih)*sina;
			pLiftVertexArray[iv++] = Pt.y + amp * sin(dih);
			pLiftVertexArray[iv++] = Pt.z + amp * cos(dih)*cosa;
		}
	}
	else
	{
		i = 0;
		int iv=0;
		//lift lines
		for (j=0; j<pWing->m_Surface.size(); j++)
		{
			for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
			{
				pWing->m_Surface[j]->getLeadingPt(k, C);
				amp = pWing->m_Surface[j]->chord(k) / pWOpp->m_StripArea[i] / pWing->m_MAChord * QMiarex::s_LiftScale/1000.0;
				C.x += pWOpp->m_XCPSpanRel[i] * pWing->m_Surface[j]->chord(k);

				pLiftVertexArray[iv++] = C.x;
				pLiftVertexArray[iv++] = C.y;
				pLiftVertexArray[iv++] = C.z;

				pLiftVertexArray[iv++] = C.x + pWOpp->m_F[i].x*amp;
				pLiftVertexArray[iv++] = C.y + pWOpp->m_F[i].y*amp;
				pLiftVertexArray[iv++] = C.z + pWOpp->m_F[i].z*amp;
				i++;
			}
		}
		//Lift strip on each surface
		i = 0;
		for (j=0; j<pWing->m_Surface.size(); j++)
		{
/*			if(j>0 && pWing->m_Surface[j-1]->m_bJoinRight)
			{
				//then connect strip to previous surface's last point
				pLiftVertexArray[iv++] = CL.x;
				pLiftVertexArray[iv++] = CL.y;
				pLiftVertexArray[iv++] = CL.z;

				k=0;
				pWing->m_Surface[j]->getLeadingPt(k, C);
				amp = pWing->m_Surface[j]->chord(k) / pWOpp->m_StripArea[i] / pWing->m_MAChord * QMiarex::s_LiftScale/1000.0;
				C.x += pWOpp->m_XCPSpanRel[i] * pWing->m_Surface[j]->chord(k);

				pLiftVertexArray[iv++] = C.x + pWOpp->m_F[i].x*amp;
				pLiftVertexArray[iv++] = C.y + pWOpp->m_F[i].y*amp;
				pLiftVertexArray[iv++] = C.z + pWOpp->m_F[i].z*amp;
			}*/

			for (k=0; k< pWing->m_Surface[j]->m_NYPanels; k++)
			{
				pWing->m_Surface[j]->getLeadingPt(k, C);
				amp = pWing->m_Surface[j]->chord(k) / pWOpp->m_StripArea[i] / pWing->m_MAChord * QMiarex::s_LiftScale/1000.0;
				C.x += pWOpp->m_XCPSpanRel[i] * pWing->m_Surface[j]->chord(k);
				CL.x = C.x + pWOpp->m_F[i].x*amp;
				CL.y = C.y + pWOpp->m_F[i].y*amp;
				CL.z = C.z + pWOpp->m_F[i].z*amp;

				pLiftVertexArray[iv++] = CL.x;
				pLiftVertexArray[iv++] = CL.y;
				pLiftVertexArray[iv++] = CL.z;
				i++;
			}
		}
	}

	m_vboLift[iWing].destroy();
	m_vboLift[iWing].create();
	m_vboLift[iWing].bind();
	m_vboLift[iWing].allocate(pLiftVertexArray, m_Ny[iWing]*9 * sizeof(GLfloat));
	m_vboLift[iWing].release();
	delete [] pLiftVertexArray;
}



void GL3Widget::paintLift(int iWing)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_XCPColor);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	m_vboLift[iWing].bind();
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_LINE_STIPPLE);

	switch(W3dPrefsDlg::s_XCPStyle)
	{
		case 1: glLineStipple (1, 0xCFCF); break;
		case 2: glLineStipple (1, 0x6666); break;
		case 3: glLineStipple (1, 0xFF18); break;
		case 4: glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}

	glLineWidth((GLfloat)W3dPrefsDlg::s_XCPWidth);

	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);

	glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
	glDrawArrays(GL_LINE_STRIP, m_Ny[iWing]*2, m_Ny[iWing]);
	m_vboLift[iWing].release();

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}



void GL3Widget::paintBody(Body *pBody)
{
	if(!pBody) return;
	int pos = 0;
	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.enableAttributeArray(m_VertexLocationTexture);
	m_ShaderProgramTexture.enableAttributeArray(m_NormalLocationTexture);
	m_ShaderProgramTexture.enableAttributeArray(m_UVLocationTexture);

	m_vboBody.bind();
	m_ShaderProgramTexture.setAttributeBuffer(m_VertexLocationTexture, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
	m_ShaderProgramTexture.setAttributeBuffer(m_NormalLocationTexture, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
	m_ShaderProgramTexture.setAttributeBuffer(m_UVLocationTexture,     GL_FLOAT, 6* sizeof(GLfloat), 2, 8 * sizeof(GLfloat));

	if(m_bSurfaces)
	{
		m_ShaderProgramTexture.setUniformValue(m_ColorLocationTexture, pBody->bodyColor());
		m_ShaderProgramTexture.setUniformValue(m_TextureLocationTexture, pBody->textures());
		m_pRightBodyTexture->bind();
		glDrawElements(GL_TRIANGLES, m_iBodyElems*3/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray);
		m_pRightBodyTexture->release();
		m_pLeftBodyTexture->bind();
		glDrawElements(GL_TRIANGLES, m_iBodyElems*3/2, GL_UNSIGNED_SHORT, m_BodyIndicesArray+m_iBodyElems*3/2);
		m_pLeftBodyTexture->release();
	}

	m_ShaderProgramTexture.disableAttributeArray(m_VertexLocationTexture);
	m_ShaderProgramTexture.disableAttributeArray(m_NormalLocationTexture);
	m_ShaderProgramTexture.disableAttributeArray(m_UVLocationTexture);
	m_ShaderProgramTexture.release();

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_OutlineColor);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	if(m_bOutline)
	{
		pos = (NX+1) * (NH+1) * 2;
		for(int iFr=0; iFr<pBody->frameCount(); iFr++)
		{
			glDrawArrays(GL_LINE_STRIP, pos, (NH+1)*2);
			pos += (NH+1)*2;
		}
		glDrawArrays(GL_LINE_STRIP, pos, NX+1);
		pos += NX+1;
		glDrawArrays(GL_LINE_STRIP, pos, NX+1);
	}

	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_vboBody.release();
	m_ShaderProgramLine.release();
}


void GL3Widget::paintWing(int iWing, Wing *pWing)
{
	if(!pWing) return;
	unsigned short *wingIndicesArray = m_WingIndicesArray[iWing];

	int pos = 0;
	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.enableAttributeArray(m_VertexLocationTexture);
	m_ShaderProgramTexture.enableAttributeArray(m_NormalLocationTexture);
	m_ShaderProgramTexture.enableAttributeArray(m_UVLocationTexture);

	m_vboWing[iWing].bind();
	m_ShaderProgramTexture.setAttributeBuffer(m_VertexLocationTexture, GL_FLOAT, 0,                  3, 8 * sizeof(GLfloat));
	m_ShaderProgramTexture.setAttributeBuffer(m_NormalLocationTexture, GL_FLOAT, 3* sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
	m_ShaderProgramTexture.setAttributeBuffer(m_UVLocationTexture,     GL_FLOAT, 6* sizeof(GLfloat), 2, 8 * sizeof(GLfloat));
	m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, m_pvmMatrix);

	if(m_bSurfaces)
	{
		m_ShaderProgramTexture.setUniformValue(m_TextureLocationTexture, pWing->textures());
		m_ShaderProgramTexture.setUniformValue(m_ColorLocationTexture, pWing->wingColor());

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);

		if(!pWing->textures())
		{
			//draw them all at once
			glDrawElements(GL_TRIANGLES, m_iWingElems[iWing], GL_UNSIGNED_SHORT, wingIndicesArray);
		}
		else
		{
			//indices array size:
			//  Top & bottom surfaces
			//      NSurfaces
			//      x (ChordPoints-1)quads
			//      x2 triangles per/quad
			//      x2 top and bottom surfaces
			//      x3 indices/triangle
			// m_iWingElems[iWing] =  pWing->m_Surface.count()* (CHORDPOINTS-1) *2 *2 *3;

			int pos = 0;
			for (int j=0; j<pWing->m_Surface.count(); j++)
			{
				//topsurface
				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingTopLeftTexture->bind();
				else                                     m_pWingTopRightTexture->bind();
				glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray+pos);
				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingTopLeftTexture->release();
				else                                     m_pWingTopRightTexture->release();
				pos += (CHORDPOINTS-1)*2*3;

				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingBotLeftTexture->bind();
				else                                     m_pWingBotRightTexture->bind();
				glDrawElements(GL_TRIANGLES, (CHORDPOINTS-1)*2*3, GL_UNSIGNED_SHORT, wingIndicesArray+pos);
				if(pWing->m_Surface.at(j)->isLeftSurf()) m_pWingBotLeftTexture->release();
				else                                     m_pWingBotRightTexture->release();
				pos += (CHORDPOINTS-1)*2*3;
			}
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	m_ShaderProgramTexture.disableAttributeArray(m_VertexLocationTexture);
	m_ShaderProgramTexture.disableAttributeArray(m_NormalLocationTexture);
	m_ShaderProgramTexture.release();

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 8 * sizeof(GLfloat));
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_OutlineColor);
	m_ShaderProgramLine.setUniformValue(m_MatrixLocationLine, m_pvmMatrix);

	if(m_bOutline)
	{
		m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_OutlineColor);
		glLineWidth(W3dPrefsDlg::s_OutlineWidth);
		glEnable (GL_LINE_STIPPLE);
		switch(W3dPrefsDlg::s_OutlineStyle)
		{
			case 1:  glLineStipple (1, 0xCFCF); break;
			case 2:  glLineStipple (1, 0x6666); break;
			case 3:  glLineStipple (1, 0xFF18); break;
			case 4:  glLineStipple (1, 0x7E66); break;
			default: glLineStipple (1, 0xFFFF); break;
		}
		glLineWidth(W3dPrefsDlg::s_VLMWidth);


		for(int iSurf=0; iSurf<pWing->m_Surface.count(); iSurf++)
		{
			//top left side
			glDrawArrays(GL_LINE_STRIP, pos, CHORDPOINTS);
			pos += CHORDPOINTS;
			//top right side
			glDrawArrays(GL_LINE_STRIP, pos, CHORDPOINTS);
			pos += CHORDPOINTS;
			//bot left side
			glDrawArrays(GL_LINE_STRIP, pos, CHORDPOINTS);
			pos += CHORDPOINTS;
			//bot right side
			glDrawArrays(GL_LINE_STRIP, pos, CHORDPOINTS);
			pos += CHORDPOINTS;
		}

		for(int iSurf=0; iSurf<pWing->m_Surface.count(); iSurf++)
		{
			glDrawArrays(GL_LINES, pos, 4*pWing->m_Surface.count());
			pos +=4;
		}
	}
	glDisable(GL_LINE_STIPPLE);
	m_vboWing[iWing].release();
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}



/**
*Creates the OpenGL List for the ArcBall.
*@param ArcBall the ArcBall object associated to the view
*@param GLScale the overall scaling factor for the view
*/
void GL3Widget::glMakeArcBall()
{
	float GLScale = 1.0f;
	int row, col;
	double Radius=0.1, lat_incr, lon_incr, phi, theta;
	CVector eye(0.0, 0.0, 1.0);
	CVector up(0.0, 1.0, 0.0);
	m_ArcBall.setZoom(0.45, eye, up);

	Radius = m_ArcBall.ab_sphere;
	lat_incr =  90.0 / (double)NUMANGLES;
	lon_incr = 360.0 / (double)NUMCIRCLES;

	int iv=0;

	float *arcBallVertexArray  = new float[((NUMCIRCLES*2)*(NUMANGLES-2) + (NUMPERIM-1)*2)*3];

	//ARCBALL
	for (col=0; col<NUMCIRCLES; col++)
	{
		//first
		phi = (col * lon_incr) * PI/180.0;
		for (row=1; row<NUMANGLES-1; row++)
		{
			theta = (row * lat_incr) * PI/180.0;
			arcBallVertexArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
		}
	}

	for (col=0; col<NUMCIRCLES; col++)
	{
		//Second
		phi = (col * lon_incr ) * PI/180.0;
		for (row=1; row<NUMANGLES-1; row++)
		{
			theta = -(row * lat_incr) * PI/180.0;
			arcBallVertexArray[iv++] = Radius*cos(phi)*cos(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(theta)*GLScale;
			arcBallVertexArray[iv++] = Radius*sin(phi)*cos(theta)*GLScale;
		}
	}

	theta = 0.;
	for(col=1; col<NUMPERIM; col++)
	{
		phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
		arcBallVertexArray[iv++] = Radius * cos(phi) * cos(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(phi) * cos(theta)*GLScale;
	}

	theta = 0.;
	for(col=1; col<NUMPERIM; col++)
	{
		phi = (0.0 + (double)col*360.0/72.0) * PI/180.0;
		arcBallVertexArray[iv++] = Radius * cos(-phi) * cos(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(theta)*GLScale;
		arcBallVertexArray[iv++] = Radius * sin(-phi) * cos(theta)*GLScale;
	}

	m_vboArcBall.destroy();
	m_vboArcBall.create();
	m_vboArcBall.bind();
	m_vboArcBall.allocate(arcBallVertexArray, iv * sizeof(GLfloat));
	m_vboArcBall.release();

	delete [] arcBallVertexArray;
}

#define NUMLONG  43
#define NUMLAT   37

/**
Creates a list for a sphere with unit radius
*/
void GL3Widget::glMakeUnitSphere()
{
	double start_lat, start_lon,lat_incr, lon_incr;
	double phi, theta;
	int iLat, iLong;

	start_lat = -90 * PI/180.0;
	start_lon = 0.0 * PI/180.0;

	lat_incr = 180.0 / (NUMLAT-1) * PI/180.0;
	lon_incr = 360.0 / (NUMLONG-1) * PI/180.0;

	GLfloat *sphereVertexArray = new GLfloat[NUMLONG * NUMLAT * 2 *3];
	m_SphereIndicesArray  = new unsigned short[NUMLONG * NUMLAT * 2];

	int iv = 0;

	for (iLong=0; iLong<NUMLONG; iLong++)
	{
		phi = (start_lon + iLong * lon_incr) ;
		for (iLat=0; iLat<NUMLAT; iLat++)
		{
			theta = (start_lat + iLat * lat_incr);
			// the point
			sphereVertexArray[iv++] = cos(phi) * cos(theta);//x
			sphereVertexArray[iv++] = sin(phi) * cos(theta);//z
			sphereVertexArray[iv++] = sin(theta);//y
			 //the normal
			sphereVertexArray[iv++] = cos(phi) * cos(theta);//x
			sphereVertexArray[iv++] = sin(phi) * cos(theta);//z
			sphereVertexArray[iv++] = sin(theta);//y
		}
	}

	int in=0;
	for (iLong=0; iLong<NUMLONG-1; iLong++)
	{
		for (iLat=0; iLat<NUMLAT; iLat++)
		{
			m_SphereIndicesArray[in++] =   iLong   *NUMLAT  + iLat;
			m_SphereIndicesArray[in++] =  (iLong+1)*NUMLAT  + iLat;
		}
	}

	m_vboSphere.create();
	m_vboSphere.bind();
	m_vboSphere.allocate(sphereVertexArray, iv * sizeof(GLfloat));
	m_vboSphere.release();
	delete [] sphereVertexArray;
}



void GL3Widget::paintSphere(CVector place, double radius, QColor sphereColor, bool bLight)
{
	m_ShaderProgramTexture.bind();
	m_ShaderProgramTexture.enableAttributeArray(m_VertexLocationTexture);
	m_ShaderProgramTexture.enableAttributeArray(m_NormalLocationTexture);

	m_vboSphere.bind();
	m_ShaderProgramTexture.setAttributeBuffer(m_VertexLocationTexture, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	m_ShaderProgramTexture.setAttributeBuffer(m_NormalLocationTexture, GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));
	m_ShaderProgramTexture.setUniformValue(m_LightLocationTexture, bLight);

	QMatrix4x4 mSphere, pvmSphere;
	mSphere.setToIdentity(); //redundant with constructor
	mSphere.translate(place.x, place.y, place.z);
	mSphere.scale(radius);

	pvmSphere = m_pvmMatrix * mSphere;

	m_ShaderProgramTexture.setUniformValue(m_pvmMatrixLocationTexture, pvmSphere);


	m_ShaderProgramTexture.setUniformValue(m_ColorLocationTexture, sphereColor);
	for(int iLong=0; iLong<NUMLONG-1; iLong++)
	{
		glDrawElements(GL_TRIANGLE_STRIP, NUMLAT*2, GL_UNSIGNED_SHORT, m_SphereIndicesArray+iLong*NUMLAT*2);
	}

	m_vboSphere.release();
	m_ShaderProgramTexture.disableAttributeArray(m_VertexLocationTexture);
	m_ShaderProgramTexture.disableAttributeArray(m_NormalLocationTexture);
	m_ShaderProgramTexture.release();
}



void GL3Widget::glMakePanelCP(QOpenGLBuffer &vbo, int nPanels, int nNodes, CVector *pNode, Panel *pPanel, PlaneOpp *pPOpp)
{
	if(!pPanel || !pNode || !nPanels) return;

	int pp, n, averageInf, averageSup, average100;

	double color;
	double lmin, lmax, range;
	double *tab = NULL;
	if(pPOpp) tab= pPOpp->m_dCp;

	CVector TA,LA, TB, LB;

	double *CpInf = new double[2*Objects3D::s_MaxPanelSize];
	double *CpSup = new double[2*Objects3D::s_MaxPanelSize];
	double *Cp100 = new double[2*Objects3D::s_MaxPanelSize];

    lmin = 10000.0;
	lmax = -10000.0;
	// find min and max Cp for scale set
	for (n=0; n<nNodes; n++)
	{
		if(pPOpp)
		{
			averageInf = 0; averageSup = 0; average100 = 0;
			CpInf[n] = 0.0; CpSup[n] = 0.0; Cp100[n] = 0.0;
			for (pp=0; pp< nPanels; pp++)
			{
				if (pNode[pPanel[pp].m_iLA].isSame(pNode[n]) || pNode[pPanel[pp].m_iTA].isSame(pNode[n]) ||
					pNode[pPanel[pp].m_iTB].isSame(pNode[n]) || pNode[pPanel[pp].m_iLB].isSame(pNode[n]))
				{
					if(pPanel[pp].m_Pos==TOPSURFACE)
					{
						CpSup[n] +=tab[pp];
						averageSup++;
					}
					else if(pPanel[pp].m_Pos<=MIDSURFACE)
					{
						CpInf[n] +=tab[pp];
						averageInf++;
					}
					else if(pPanel[pp].m_Pos==BODYSURFACE)
					{
						Cp100[n] +=tab[pp];
						average100++;
					}
				}
			}
			if(averageSup>0)
			{
				CpSup[n] /= averageSup;
				if(CpSup[n]<lmin) lmin = CpSup[n];
				if(lmax<CpSup[n]) lmax = CpSup[n];
			}
			if(averageInf>0)
			{
				CpInf[n] /= averageInf;
				if(CpInf[n]<lmin) lmin = CpInf[n];
				if(lmax<CpInf[n]) lmax = CpInf[n];
			}
			if(average100>0)
			{
				Cp100[n] /= average100;
				if(Cp100[n]<lmin) lmin = Cp100[n];
				if(lmax<Cp100[n]) lmax = Cp100[n];
			}

			if(QMiarex::s_bAutoCpScale)
			{
				QMiarex::s_LegendMin = lmin;
				QMiarex::s_LegendMax = lmax;
			}
			else
			{
				lmin = QMiarex::s_LegendMin;
				lmax = QMiarex::s_LegendMax;
			}
		}
	}


	range = lmax - lmin;

	// unfortunately we can't just use nodes and colors, because the trailing edges are merged
	// and the colors would be mixed
	// so write as many nodes as there are triangles.
	//
	// vertices array size:
	//		nPanels
	//      x2 triangles per panels
	//      x3 nodes per triangle
	//		x6 = 3 vertex components + 3 color components

	int nodeVertexSize = nPanels * 2 * 3 * 6;
	float *nodeVertexArray = new float[nodeVertexSize];

	Q_ASSERT(nPanels==Objects3D::s_MatSize);

	int iv=0;
	for (int p=0; p<nPanels; p++)
	{
		TA.copy(pNode[pPanel[p].m_iTA]);
		TB.copy(pNode[pPanel[p].m_iTB]);
		LA.copy(pNode[pPanel[p].m_iLA]);
		LB.copy(pNode[pPanel[p].m_iLB]);
		// each quad is two triangles
		// write the first one
		nodeVertexArray[iv++] = TA.x;
		nodeVertexArray[iv++] = TA.y;
		nodeVertexArray[iv++] = TA.z;
		if(pPOpp)
		{
			if(pPanel[p].m_Pos==TOPSURFACE)      color = (CpSup[pPanel[p].m_iTA]-lmin)/range;
			else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iTA]-lmin)/range;
			else                                 color = (Cp100[pPanel[p].m_iTA]-lmin)/range;
			nodeVertexArray[iv++] = GLGetRed(color);
			nodeVertexArray[iv++] = GLGetGreen(color);
			nodeVertexArray[iv++] = GLGetBlue(color);
		}
		else
		{
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.redF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.greenF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.blueF();
		}

		nodeVertexArray[iv++] = LA.x;
		nodeVertexArray[iv++] = LA.y;
		nodeVertexArray[iv++] = LA.z;
		if(pPOpp)
		{

			if(pPanel[p].m_Pos==TOPSURFACE)      color = (CpSup[pPanel[p].m_iLA]-lmin)/range;
			else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iLA]-lmin)/range;
			else                                 color = (Cp100[pPanel[p].m_iLA]-lmin)/range;
			nodeVertexArray[iv++] = GLGetRed(color);
			nodeVertexArray[iv++] = GLGetGreen(color);
			nodeVertexArray[iv++] = GLGetBlue(color);
		}
		else
		{
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.redF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.greenF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.blueF();
		}

		nodeVertexArray[iv++] = LB.x;
		nodeVertexArray[iv++] = LB.y;
		nodeVertexArray[iv++] = LB.z;
		if(pPOpp)
		{
			if(pPanel[p].m_Pos==TOPSURFACE)      color = (CpSup[pPanel[p].m_iLB]-lmin)/range;
			else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iLB]-lmin)/range;
			else                                 color = (Cp100[pPanel[p].m_iLB]-lmin)/range;
			nodeVertexArray[iv++] = GLGetRed(color);
			nodeVertexArray[iv++] = GLGetGreen(color);
			nodeVertexArray[iv++] = GLGetBlue(color);
		}
		else
		{
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.redF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.greenF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.blueF();
		}

		// write the second one
		nodeVertexArray[iv++] = LB.x;
		nodeVertexArray[iv++] = LB.y;
		nodeVertexArray[iv++] = LB.z;

		if(pPOpp)
		{
			if(pPanel[p].m_Pos==TOPSURFACE)      color = (CpSup[pPanel[p].m_iLB]-lmin)/range;
			else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iLB]-lmin)/range;
			else                                 color = (Cp100[pPanel[p].m_iLB]-lmin)/range;
			nodeVertexArray[iv++] = GLGetRed(color);
			nodeVertexArray[iv++] = GLGetGreen(color);
			nodeVertexArray[iv++] = GLGetBlue(color);
		}
		else
		{
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.redF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.greenF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.blueF();
		}

		nodeVertexArray[iv++] = TB.x;
		nodeVertexArray[iv++] = TB.y;
		nodeVertexArray[iv++] = TB.z;
		if(pPOpp)
		{
			if(pPanel[p].m_Pos==TOPSURFACE)      color = (CpSup[pPanel[p].m_iTB]-lmin)/range;
			else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iTB]-lmin)/range;
			else                                 color = (Cp100[pPanel[p].m_iTB]-lmin)/range;
			nodeVertexArray[iv++] = GLGetRed(color);
			nodeVertexArray[iv++] = GLGetGreen(color);
			nodeVertexArray[iv++] = GLGetBlue(color);
		}
		else
		{
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.redF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.greenF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.blueF();
		}

		nodeVertexArray[iv++] = TA.x;
		nodeVertexArray[iv++] = TA.y;
		nodeVertexArray[iv++] = TA.z;
		if(pPOpp)
		{
			if(pPanel[p].m_Pos==TOPSURFACE)      color = (CpSup[pPanel[p].m_iTA]-lmin)/range;
			else if(pPanel[p].m_Pos<=MIDSURFACE) color = (CpInf[pPanel[p].m_iTA]-lmin)/range;
			else                                 color = (Cp100[pPanel[p].m_iTA]-lmin)/range;
			nodeVertexArray[iv++] = GLGetRed(color);
			nodeVertexArray[iv++] = GLGetGreen(color);
			nodeVertexArray[iv++] = GLGetBlue(color);
		}
		else
		{
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.redF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.greenF();
			nodeVertexArray[iv++] = Settings::s_BackgroundColor.blueF();
		}
	}

	Q_ASSERT(iv==nodeVertexSize);
	Q_ASSERT(iv==Objects3D::s_MatSize*2*3*6);

	vbo.destroy();
	vbo.create();
	vbo.bind();
	vbo.allocate(nodeVertexArray, nodeVertexSize * sizeof(GLfloat));
	vbo.release();

	delete [] nodeVertexArray;
	delete [] CpInf;
	delete [] CpSup;
	delete [] Cp100;
}


void GL3Widget::glMakeWing(int iWing, Wing *pWing, Body *pBody)
{
	int j, l ;
	CVector N;
	CVector NormalA[CHORDPOINTS], NormalB[CHORDPOINTS];
	CVector PtBotLeft[CHORDPOINTS], PtBotRight[CHORDPOINTS], PtTopLeft[CHORDPOINTS], PtTopRight[CHORDPOINTS];
	double leftV[CHORDPOINTS], rightV[CHORDPOINTS];
	double leftU=0.0, rightU=1.0;
	memset(NormalA, 0, sizeof(NormalA));
	memset(NormalB, 0, sizeof(NormalB));
	//vertices array size:
	// surface:
	//     pWing->NSurfaces
	//     xCHORDPOINTS : from 0 to CHORDPOINTS
	//     x2  for A and B sides
	//     x2  for top and bottom
	// outline
	//     2 points mLA & mLB for leading edge
	//     2 points mTA & mTB for trailing edge
	//
	// x8  : for 3 vertex components, 3 normal components, 2 texture components
	/**    @todo flap lines*/

	int bufferSize = pWing->m_Surface.count()*CHORDPOINTS*2*2 + pWing->m_Surface.size()*2*2;
	bufferSize *= 8;

	float *wingVertexArray = new float[bufferSize];

	N.set(0.0, 0.0, 0.0);
	int iv=0; //index of vertex components

	//SURFACE
	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		//top surface
		pWing->m_Surface.at(j)->getSidePoints(TOPSURFACE, pBody, PtTopLeft, PtTopRight, NormalA, NormalB, CHORDPOINTS);
		pWing->getTextureUV(j, leftV, rightV, leftU, rightU, CHORDPOINTS);

//		for(int i=0; i<CHORDPOINTS; i++) qDebug("%13.7f  %13.7f  ",	leftU[i],rightU[i]);
		//left side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtTopLeft[l].x;
			wingVertexArray[iv++] = PtTopLeft[l].y;
			wingVertexArray[iv++] = PtTopLeft[l].z;
			wingVertexArray[iv++] = NormalA[l].x;
			wingVertexArray[iv++] = NormalA[l].y;
			wingVertexArray[iv++] = NormalA[l].z;
			wingVertexArray[iv++] = leftU;
			wingVertexArray[iv++] = leftV[l];
		}
		//right side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtTopRight[l].x;
			wingVertexArray[iv++] = PtTopRight[l].y;
			wingVertexArray[iv++] = PtTopRight[l].z;
			wingVertexArray[iv++] = NormalB[l].x;
			wingVertexArray[iv++] = NormalB[l].y;
			wingVertexArray[iv++] = NormalB[l].z;
			wingVertexArray[iv++] = rightU;
			wingVertexArray[iv++] = rightV[l];
		}


		//bottom surface
		pWing->m_Surface.at(j)->getSidePoints(BOTSURFACE, pBody, PtBotLeft, PtBotRight, NormalA, NormalB, CHORDPOINTS);

		//left side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtBotLeft[l].x;
			wingVertexArray[iv++] = PtBotLeft[l].y;
			wingVertexArray[iv++] = PtBotLeft[l].z;
			wingVertexArray[iv++] = NormalA[l].x;
			wingVertexArray[iv++] = NormalA[l].y;
			wingVertexArray[iv++] = NormalA[l].z;
			wingVertexArray[iv++] = 1.0f-leftU;
			wingVertexArray[iv++] = leftV[l];
		}

		//right side vertices
		for (l=0; l<CHORDPOINTS; l++)
		{
			wingVertexArray[iv++] = PtBotRight[l].x;
			wingVertexArray[iv++] = PtBotRight[l].y;
			wingVertexArray[iv++] = PtBotRight[l].z;
			wingVertexArray[iv++] = NormalB[l].x;
			wingVertexArray[iv++] = NormalB[l].y;
			wingVertexArray[iv++] = NormalB[l].z;
			wingVertexArray[iv++] = 1.0f-rightU;
			wingVertexArray[iv++] = rightV[l];
		}

	}

	for (j=0; j<pWing->m_Surface.size(); j++)
	{
		//leading edge
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_LA.x;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_LA.y;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_LA.z;
		wingVertexArray[iv++] = -1.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] = 0;
		wingVertexArray[iv++] = 0;

		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_LB.x;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_LB.y;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_LB.z;
		wingVertexArray[iv++] = -1.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] = 0;
		wingVertexArray[iv++] = 0;

		//trailing edge
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_TA.x;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_TA.y;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_TA.z;
		wingVertexArray[iv++] = +1.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] = 0;
		wingVertexArray[iv++] = 0;

		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_TB.x;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_TB.y;
		wingVertexArray[iv++] = pWing->m_Surface.at(j)->m_TB.z;
		wingVertexArray[iv++] = +1.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] =  0.0;
		wingVertexArray[iv++] = 0;
		wingVertexArray[iv++] = 0;
	}


	//indices array size:
	//  Top & bottom surfaces
	//      NSurfaces
	//      x (ChordPoints-1)quads
	//      x2 triangles per/quad
	//      x2 top and bottom surfaces
	//      x3 indices/triangle
	//  Tip patches
	//      (CHORDPOINTS-1) quads
	//      x2 triangles per/quad
	//      x2 tip patches
	//      x3 indices/triangle

	m_iWingElems[iWing] =  pWing->m_Surface.count()* (CHORDPOINTS-1) *2 *2 *3 + (CHORDPOINTS-1) *2 *2 *3;
	if(m_WingIndicesArray[iWing]) delete[] m_WingIndicesArray[iWing];
	m_WingIndicesArray[iWing] = new unsigned short[m_iWingElems[iWing]];
	unsigned short *wingIndicesArray = m_WingIndicesArray[iWing];
	int ii = 0;
	int nV=0;
	for (j=0; j<pWing->m_Surface.count(); j++)
	{
		//topsurface
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			//first triangle
			wingIndicesArray[ii]   = nV;
			wingIndicesArray[ii+1] = nV+1;
			wingIndicesArray[ii+2] = nV+CHORDPOINTS;
			//second triangle
			wingIndicesArray[ii+3] = nV+CHORDPOINTS;
			wingIndicesArray[ii+4] = nV+1;
			wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
			ii += 6;
			nV++;
		}
		nV +=CHORDPOINTS+1;

		//botsurface
		for (l=0; l<CHORDPOINTS-1; l++)
		{
			//first triangle
			wingIndicesArray[ii]   = nV;
			wingIndicesArray[ii+1] = nV+1;
			wingIndicesArray[ii+2] = nV+CHORDPOINTS;
			//second triangle
			wingIndicesArray[ii+3] = nV+CHORDPOINTS;
			wingIndicesArray[ii+4] = nV+1;
			wingIndicesArray[ii+5] = nV+CHORDPOINTS+1;
			ii += 6;
			nV++;
		}
		nV +=CHORDPOINTS+1;
	}

	//TIP PATCHES
	nV=0;
	for (j=0; j<pWing->m_Surface.count(); j++)
	{
		if(pWing->m_Surface.at(j)->isTipLeft())
		{
			for (l=0; l<CHORDPOINTS-1; l++)
			{
				//first triangle
				wingIndicesArray[ii]   = nV;
				wingIndicesArray[ii+1] = nV+1;
				wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;
				//second triangle
				wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
				wingIndicesArray[ii+4] = nV+1;
				wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
				ii += 6;
				nV++; //move one vertex
			}
			nV++; //skip the last vertex
		}

		if(pWing->m_Surface.at(j)->isTipRight())
		{
			for (l=0; l<CHORDPOINTS-1; l++)
			{
				//first triangle
				wingIndicesArray[ii]   = nV;
				wingIndicesArray[ii+1] = nV+1;
				wingIndicesArray[ii+2] = nV+2*CHORDPOINTS;

				//second triangle
				wingIndicesArray[ii+3] = nV+2*CHORDPOINTS;
				wingIndicesArray[ii+4] = nV+1;
				wingIndicesArray[ii+5] = nV+2*CHORDPOINTS+1;
				ii += 6;
				nV++; //move one vertex
			}
			nV++; //skip the last vertex;
			nV += CHORDPOINTS; //skip the bottom line of this wing section
		}

		if(pWing->m_Surface.at(j)->isTipLeft())
		{
			nV+= 3*CHORDPOINTS;
		}
		else if(pWing->m_Surface.at(j)->isTipRight())
		{
		}
		else
		{
			nV +=4*CHORDPOINTS;
		}
	}

	m_pWingTopLeftTexture  = new QOpenGLTexture(QImage(QString(":/images/wing_top_left.png")));
	m_pWingTopRightTexture = new QOpenGLTexture(QImage(QString(":/images/wing_top_right.png")));
	m_pWingBotLeftTexture  = new QOpenGLTexture(QImage(QString(":/images/wing_bottom_left.png")));
	m_pWingBotRightTexture = new QOpenGLTexture(QImage(QString(":/images/wing_bottom_right.png")));

	m_vboWing[iWing].destroy();
	m_vboWing[iWing].create();
	m_vboWing[iWing].bind();
	m_vboWing[iWing].allocate(wingVertexArray, bufferSize * sizeof(GLfloat));
	m_vboWing[iWing].release();

	delete[] wingVertexArray;
}




/** Used in wing edition only */
void GL3Widget::glMakeWingMesh(Wing *pWing)
{
	int l,k;

	//not necessarily the same Nx for all surfaces, so we need to count panels
	int bufferSize = 0;
	for (int j=0; j<pWing->m_Surface.size(); j++)
	{
		//chordwise lines
		bufferSize += (pWing->m_Surface.at(j)->m_NXPanels+1) * (pWing->m_Surface.at(j)->m_NYPanels+1) *2;

		//spanwise lines
		bufferSize += (pWing->m_Surface.at(j)->m_NXPanels+1) * (pWing->m_Surface.at(j)->m_NYPanels+1) *2;

		//tip patches
		if(pWing->m_Surface.at(j)->isTipLeft())
			bufferSize += (pWing->m_Surface.at(j)->m_NXPanels+1) *2;
		if(pWing->m_Surface.at(j)->isTipRight())
			bufferSize += (pWing->m_Surface.at(j)->m_NXPanels+1) *2;

		//add background triangle strips
		bufferSize += (pWing->m_Surface.at(j)->m_NXPanels+1)*2 * (pWing->m_Surface.at(j)->m_NYPanels) *2;
	}
	bufferSize *=3; //3 vertices for each node

	float *meshVertexArray = new float[bufferSize];

	int iv=0;

	for (int j=0; j<pWing->m_Surface.size(); j++)
	{
		//top surface chordwise lines
		for(k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
		{
			for (int l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				pWing->m_Surface.at(j)->getPanel(k,l,TOPSURFACE);
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.z;
			}
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.z;
		}
		k--;
		for (l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
		{
			pWing->m_Surface.at(j)->getPanel(k,l,TOPSURFACE);
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.z;
		}
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.x;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.y;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.z;

		//bot surface chordwise lines
		for(k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
		{
			for (l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				pWing->m_Surface.at(j)->getPanel(k,l,BOTSURFACE);
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.z;
			}
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.z;
		}
		k--;
		for (l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
		{
			pWing->m_Surface.at(j)->getPanel(k,l,BOTSURFACE);
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.z;
		}
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.x;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.y;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.z;


		//top surface spanwise lines
		for(l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
		{
			for (k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
			{
				pWing->m_Surface.at(j)->getPanel(k,l,TOPSURFACE);
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.z;
			}
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.z;
		}
		l--;
		for (k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
		{
			pWing->m_Surface.at(j)->getPanel(k,l,TOPSURFACE);
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.z;
		}
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.x;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.y;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.z;

		//bot surface spanwise lines
		for(l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
		{
			for (k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
			{
				pWing->m_Surface.at(j)->getPanel(k,l,BOTSURFACE);
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.z;
			}
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.z;
		}
		l--;
		for (k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
		{
			pWing->m_Surface.at(j)->getPanel(k,l,BOTSURFACE);
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.z;
		}
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.x;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.y;
		meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.z;
	}

CVector A,B,C,D;

	//tip patches
	for (int j=0; j<pWing->m_Surface.size(); j++)
	{
		if(pWing->m_Surface.at(j)->isTipLeft())
		{
			for (l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				pWing->m_Surface.at(j)->getPanel(0,l,TOPSURFACE);
				A = pWing->m_Surface.at(j)->TA;
				B = pWing->m_Surface.at(j)->LA;
				pWing->m_Surface.at(j)->getPanel(0,l,BOTSURFACE);
				C = pWing->m_Surface.at(j)->LA;
				D = pWing->m_Surface.at(j)->TA;

				meshVertexArray[iv++] = A.x;
				meshVertexArray[iv++] = A.y;
				meshVertexArray[iv++] = A.z;
				meshVertexArray[iv++] = C.x;
				meshVertexArray[iv++] = C.y;
				meshVertexArray[iv++] = C.z;
//qDebug("%13.7f   %13.7f   %13.7f   ",pWing->m_Surface.at(j)->TB.x, pWing->m_Surface.at(j)->TB.y, pWing->m_Surface.at(j)->TB.z);
			}
			meshVertexArray[iv++] = B.x;
			meshVertexArray[iv++] = B.y;
			meshVertexArray[iv++] = B.z;
			meshVertexArray[iv++] = D.x;
			meshVertexArray[iv++] = D.y;
			meshVertexArray[iv++] = D.z;
		}
		if(pWing->m_Surface.at(j)->isTipRight())
		{
			for (l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				pWing->m_Surface.at(j)->getPanel(pWing->m_Surface.at(j)->m_NYPanels-1,l,TOPSURFACE);
				A = pWing->m_Surface.at(j)->TB;
				B = pWing->m_Surface.at(j)->LB;
				pWing->m_Surface.at(j)->getPanel(pWing->m_Surface.at(j)->m_NYPanels-1,l,BOTSURFACE);
				C = pWing->m_Surface.at(j)->LB;
				D = pWing->m_Surface.at(j)->TB;

				meshVertexArray[iv++] = A.x;
				meshVertexArray[iv++] = A.y;
				meshVertexArray[iv++] = A.z;
				meshVertexArray[iv++] = C.x;
				meshVertexArray[iv++] = C.y;
				meshVertexArray[iv++] = C.z;
			}
			meshVertexArray[iv++] = B.x;
			meshVertexArray[iv++] = B.y;
			meshVertexArray[iv++] = B.z;
			meshVertexArray[iv++] = D.x;
			meshVertexArray[iv++] = D.y;
			meshVertexArray[iv++] = D.z;
		}
	}


	//background surface
	for (int j=0; j<pWing->m_Surface.size(); j++)
	{
		for(k=0; k<pWing->m_Surface.at(j)->m_NYPanels; k++)
		{
			for (l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				pWing->m_Surface.at(j)->getPanel(k,l,TOPSURFACE);

				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.z;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.z;
			}
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.z;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.z;

			for (l=0; l<pWing->m_Surface.at(j)->m_NXPanels; l++)
			{
				pWing->m_Surface.at(j)->getPanel(k,l,BOTSURFACE);
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TA.z;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.x;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.y;
				meshVertexArray[iv++] = pWing->m_Surface.at(j)->TB.z;
			}
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LA.z;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.x;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.y;
			meshVertexArray[iv++] = pWing->m_Surface.at(j)->LB.z;
		}
	}



//	Q_ASSERT(iv==bufferSize);

//	m_iWingMeshElems = ii/3;
	m_vboWingMesh.destroy();
	m_vboWingMesh.create();
	m_vboWingMesh.bind();
	m_vboWingMesh.allocate(meshVertexArray, bufferSize * sizeof(GLfloat));
	m_vboWingMesh.release();

	delete[] meshVertexArray;
}


void GL3Widget::glMakeCpLegendClr()
{
	int i;
	QFont fnt(Settings::s_TextFont); //valgrind
	QFontMetrics fm(fnt);
	double fmw = (double) fm.averageCharWidth();

	double fi, ZPos,dz,Right1, Right2;
	double color = 0.0;

	double w = (double)rect().width();
	double h = (double)rect().height();
	double XPos;

	if(w>h)
	{
		XPos  = 1.0;
		dz    = h/w /(float)MAXCPCOLORS/2.0;
		ZPos  = h/w/10 - 12.0*dz;
	}
	else
	{
		XPos  = w/h;
		dz    = 1. /(float)MAXCPCOLORS/2.0;
		ZPos  = 1./10 - 12.0*dz;
	}

	Right1  = XPos - 8 * fmw/w;
	Right2  = XPos - 3 * fmw/w;

	int colorLegendSize = MAXCPCOLORS*2*6;
	float *pColorVertexArray = new float[(MAXCPCOLORS+1)*2*6];

	int iv = 0;
	for (i=0; i<MAXCPCOLORS; i++)
	{
		fi = (double)i*dz;
		color = (float)i/(float)(MAXCPCOLORS-1);

		pColorVertexArray[iv++] = Right1;
		pColorVertexArray[iv++] = ZPos+2*fi;
		pColorVertexArray[iv++] = 0.0;
		pColorVertexArray[iv++] = GLGetRed(color);
		pColorVertexArray[iv++] = GLGetGreen(color);
		pColorVertexArray[iv++] = GLGetBlue(color);

		pColorVertexArray[iv++] = Right2;
		pColorVertexArray[iv++] = ZPos+2*fi;
		pColorVertexArray[iv++] = 0.0;
		pColorVertexArray[iv++] = GLGetRed(color);
		pColorVertexArray[iv++] = GLGetGreen(color);
		pColorVertexArray[iv++] = GLGetBlue(color);
	}
	m_vboLegendColor.destroy();
	m_vboLegendColor.create();
	m_vboLegendColor.bind();
	m_vboLegendColor.allocate(pColorVertexArray, colorLegendSize * sizeof(GLfloat));
	m_vboLegendColor.release();
	delete [] pColorVertexArray;
}



bool GL3Widget::glMakeStreamLines(Wing *PlaneWing[MAXWINGS], CVector *pNode, WPolar *pWPolar, PlaneOpp *pPOpp)
{
	if(!pPOpp || !pWPolar || pWPolar->analysisMethod()==XFLR5::LLTMETHOD  || !Objects3D::s_MatSize) return false;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	double memcoresize = Panel::coreSize();
	Panel::setCoreSize(0.0005); //mm, just for the time needed to build the streamlines which are very sensitive to trailing vortex interference

	Wing *pWing;

	QProgressDialog dlg(tr("Streamlines calculation"), tr("Abort"), 0, Objects3D::s_MatSize);
	dlg.setWindowModality(Qt::WindowModal);

	bool bFound;
	int i, m, p, iWing;
	double ds;
	double *Mu, *Sigma;

	CVector C, D, D1, VA, VAT, VB, VBT, VT, VInf, TC, TD;
	CVector RefPoint(0.0,0.0,0.0);

	D1.set(987654321.0, 0.0, 0.0);

	Mu    = pPOpp->m_dG;
	Sigma = pPOpp->m_dSigma;

	VInf.set(pPOpp->m_QInf,0.0,0.0);

	m = 0;

	m_NStreamLines = 0;
	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if(PlaneWing[iw]) m_NStreamLines += m_Ny[iw]+2;
	}

	int streamArraySize = 	m_NStreamLines * GL3DScales::s_NX * 3;
	float *pStreamVertexArray = new float[streamArraySize];

	int iv = 0;
	for (iWing=0; iWing<MAXWINGS; iWing++)
	{
		if(PlaneWing[iWing])
		{
			pWing = PlaneWing[iWing];
			int nVertex = 0;
			for (p=0; p<pWing->m_MatSize; p++)
			{
				bFound = false;

				if(GL3DScales::s_pos==0 && pWing->m_pWingPanel[p].m_bIsLeading && pWing->m_pWingPanel[p].m_Pos<=MIDSURFACE)
				{
					C.set(pNode[pWing->m_pWingPanel[p].m_iLA]);
					D.set(pNode[pWing->m_pWingPanel[p].m_iLB]);
					bFound = true;
				}
				else if(GL3DScales::s_pos==1 && pWing->m_pWingPanel[p].m_bIsTrailing && pWing->m_pWingPanel[p].m_Pos<=MIDSURFACE)
				{
					C.set(pNode[pWing->m_pWingPanel[p].m_iTA]);
					D.set(pNode[pWing->m_pWingPanel[p].m_iTB]);
					bFound = true;
				}
				else if(GL3DScales::s_pos==2 && pWing->m_pWingPanel[p].m_bIsLeading && pWing->m_pWingPanel[p].m_Pos<=MIDSURFACE)
				{
					C.set(0.0, pNode[pWing->m_pWingPanel[p].m_iLA].y, 0.0);
					D.set(0.0, pNode[pWing->m_pWingPanel[p].m_iLB].y, 0.0);
					bFound = true;
				}

				if(bFound)
				{
					TC = C;
					TD = D;
					TC.rotateY(RefPoint, pPOpp->alpha());
					TD.rotateY(RefPoint, pPOpp->alpha());
					TC -= C;
					TD -= D;
					if(GL3DScales::s_pos==1 && qAbs(GL3DScales::s_XOffset)<0.001 && qAbs(GL3DScales::s_ZOffset)<0.001)
					{
						//apply Kutta's condition : initial speed vector is parallel to the T.E. bisector angle
						VA.set(pNode[pWing->m_pWingPanel[p].m_iTA] - pNode[pWing->m_pWingPanel[p].m_iLA]);
						VA. normalize();
						VB.set(pNode[pWing->m_pWingPanel[p].m_iTB] - pNode[pWing->m_pWingPanel[p].m_iLB]);
						VB. normalize();
						if(pWing->m_pWingPanel[p].m_Pos==BOTSURFACE)
						{
							//corresponding upper panel is the next one coming up
							for (i=p; i<pWing->m_MatSize;i++)
								if(pWing->m_pWingPanel[i].m_Pos>MIDSURFACE && pWing->m_pWingPanel[i].m_bIsTrailing) break;
							VAT = pNode[pWing->m_pWingPanel[i].m_iTA] - pNode[pWing->m_pWingPanel[i].m_iLA];
							VAT.normalize();
							VA = VA+VAT;
							VA.normalize();//VA is parallel to the bisector angle

							VBT = pNode[pWing->m_pWingPanel[i].m_iTB] - pNode[pWing->m_pWingPanel[i].m_iLB];
							VBT.normalize();
							VB = VB+VBT;
							VB.normalize();//VB is parallel to the bisector angle
						}
						VA.rotateY(pPOpp->alpha());
						VB.rotateY(pPOpp->alpha());
					}


					if(!C.isSame(D1))
					{
						// we plot the left trailing point only for the extreme left trailing panel
						// and only right trailing points afterwards
						C.x += GL3DScales::s_XOffset;
						C.z += GL3DScales::s_ZOffset;

						ds = GL3DScales::s_DeltaL;

						// One very special case is where we initiate the streamlines exactly at the T.E.
						// without offset either in X ou Z directions
//							V1.Set(0.0,0.0,0.0);

						pStreamVertexArray[iv++] = C.x+TC.x;
						pStreamVertexArray[iv++] = C.y+TC.y;
						pStreamVertexArray[iv++] = C.z+TC.z;
						C   += VA *ds;
						pStreamVertexArray[iv++] = C.x+TC.x;
						pStreamVertexArray[iv++] = C.y+TC.y;
						pStreamVertexArray[iv++] = C.z+TC.z;
						ds *= GL3DScales::s_XFactor;
						nVertex +=2;

						for (i=2; i< GL3DScales::s_NX ;i++)
						{
							Objects3D::s_pPanelAnalysis->getSpeedVector(C, Mu, Sigma, VT);

							VT += VInf;
							VT.normalize();
							C   += VT* ds;
							pStreamVertexArray[iv++] = C.x+TC.x;
							pStreamVertexArray[iv++] = C.y+TC.y;
							pStreamVertexArray[iv++] = C.z+TC.z;
							nVertex +=1;
							ds *= GL3DScales::s_XFactor;
						}
					}

					// right trailing point
					D1 = D;
					D.x += GL3DScales::s_XOffset;
					D.z += GL3DScales::s_ZOffset;

					ds = GL3DScales::s_DeltaL;

//					V1.Set(0.0,0.0,0.0);

					pStreamVertexArray[iv++] = D.x+TD.x;
					pStreamVertexArray[iv++] = D.y+TD.y;
					pStreamVertexArray[iv++] = D.z+TD.z;
					D   += VB *ds;
					pStreamVertexArray[iv++] = D.x+TD.x;
					pStreamVertexArray[iv++] = D.y+TD.y;
					pStreamVertexArray[iv++] = D.z+TD.z;
					ds *= GL3DScales::s_XFactor;
					nVertex +=2;

					for (i=2; i<GL3DScales::s_NX; i++)
					{
						Objects3D::s_pPanelAnalysis->getSpeedVector(D, Mu, Sigma, VT);

						VT += VInf;
						VT.normalize();
						D   += VT* ds;
						pStreamVertexArray[iv++] = D.x+TD.x;
						pStreamVertexArray[iv++] = D.y+TD.y;
						pStreamVertexArray[iv++] = D.z+TD.z;
						ds *= GL3DScales::s_XFactor;
						nVertex +=1;
					}
				}

				dlg.setValue(m);
				m++;

				if(dlg.wasCanceled()) break;
			}
			if(dlg.wasCanceled()) break;
		}
		if(dlg.wasCanceled()) break;
	}

	//restore things as they were
	Panel::setCoreSize(memcoresize);
	QApplication::restoreOverrideCursor();

	m_vboStreamLines.destroy();
	m_vboStreamLines.create();
	m_vboStreamLines.bind();
	m_vboStreamLines.allocate(pStreamVertexArray, streamArraySize*sizeof(float));
	m_vboStreamLines.release();
	delete [] pStreamVertexArray;

	if(dlg.wasCanceled()) return false;
	return true;
}



void GL3Widget::glMakeTransistions(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
{
	if(!pWing || !pWPolar || !pWOpp) return;
	int i,j,k,m;
	double yrel, yob ;
	CVector Pt, N;

	float *pTransVertexArray = NULL;
	pTransVertexArray = new float[m_Ny[iWing]*6];
	int iv=0;
	if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
			pWing->surfacePoint(pWOpp->m_XTrTop[i], yob, TOPSURFACE, Pt, N);

			pTransVertexArray[iv++] = Pt.x;
			pTransVertexArray[iv++] = Pt.y;
			pTransVertexArray[iv++] = Pt.z;
		}
	}
	else
	{
		if(!pWing->isFin())
		{
			m = 0;
			for(j=0; j<pWing->m_Surface.size(); j++)
			{
				for(k=0; k<pWing->m_Surface[j]->m_NYPanels; k++)
				{
					yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
					pWing->m_Surface[j]->getSurfacePoint(pWOpp->m_XTrTop[m],pWOpp->m_XTrTop[m],yrel,TOPSURFACE,Pt,N);

					pTransVertexArray[iv++] = Pt.x;
					pTransVertexArray[iv++] = Pt.y;
					pTransVertexArray[iv++] = Pt.z;
					m++;
				}
			}
		}
		else
		{
			m = 0;
			for(j=0; j<pWing->m_Surface.size(); j++)
			{
				for(k=0; k<pWing->m_Surface[j]->m_NYPanels; k++)
				{
					yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
					pWing->m_Surface[j]->getSurfacePoint(pWOpp->m_XTrTop[m],pWOpp->m_XTrTop[m],yrel,TOPSURFACE,Pt,N);
					pTransVertexArray[iv++] = Pt.x;
					pTransVertexArray[iv++] = Pt.y;
					pTransVertexArray[iv++] = Pt.z;

					m++;
				}
			}
		}
	}

	if(pWPolar->analysisMethod()==XFLR5::LLTMETHOD)
	{
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
			pWing->surfacePoint(pWOpp->m_XTrBot[i], yob, BOTSURFACE, Pt, N);
			pTransVertexArray[iv++] = Pt.x;
			pTransVertexArray[iv++] = Pt.y;
			pTransVertexArray[iv++] = Pt.z;
		}
	}
	else
	{
		if(!pWing->isFin())
		{
			int m = 0;
			for(j=0; j<pWing->m_Surface.size(); j++)
			{
				for(k=0; k<pWing->m_Surface[j]->m_NYPanels; k++)
				{
					yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
					pWing->m_Surface[j]->getSurfacePoint(pWOpp->m_XTrBot[m],pWOpp->m_XTrBot[m],yrel,BOTSURFACE,Pt,N);
					pTransVertexArray[iv++] = Pt.x;
					pTransVertexArray[iv++] = Pt.y;
					pTransVertexArray[iv++] = Pt.z;
					m++;
				}
			}
		}
		else
		{
			int m = 0;
			for(j=0; j<pWing->m_Surface.size(); j++)
			{
				for(k=0; k<pWing->m_Surface[j]->m_NYPanels; k++)
				{
					yrel = pWing->yrel(pWOpp->m_SpanPos[m]);
					pWing->m_Surface[j]->getSurfacePoint(pWOpp->m_XTrBot[m],pWOpp->m_XTrBot[m],yrel,BOTSURFACE,Pt,N);
					pTransVertexArray[iv++] = Pt.x;
					pTransVertexArray[iv++] = Pt.y;
					pTransVertexArray[iv++] = Pt.z;
					m++;
				}
			}
		}
	}



	m_vboTransitions[iWing].destroy();
	m_vboTransitions[iWing].create();
	m_vboTransitions[iWing].bind();
	m_vboTransitions[iWing].allocate(pTransVertexArray, m_Ny[iWing]*3 * sizeof(GLfloat));
	m_vboTransitions[iWing].release();
	delete [] pTransVertexArray;
}




void GL3Widget::glMakeWingSectionHighlight(Wing *pWing, int iSectionHighLight, bool bRightSide)
{
	CVector Point, Normal;
	int iSection = 0;
	int jSurf = 0;
	for(int jSection=0; jSection<pWing->NWingSection(); jSection++)
	{
		if(jSection==iSectionHighLight) break;
		if(qAbs(pWing->YPosition(jSection+1)-pWing->YPosition(jSection)) > Wing::s_MinPanelSize)
			iSection++;
	}

	m_HighlightLineSize = CHORDPOINTS * 2;
	int bufferSize = m_HighlightLineSize *2 *3 ;
	float *pHighlightVertexArray = new float[bufferSize];

	int iv=0;
	if(iSection==0)
	{
		m_nHighlightLines = 1;
		//define the inner left side surface
		if(!pWing->isFin())  jSurf = pWing->m_Surface.size()/2 - 1;
		else                 jSurf = pWing->m_Surface.size()   - 1;

		for (int lx=0; lx<CHORDPOINTS; lx++)
		{
			double xRel = (double)lx/(double)(CHORDPOINTS-1);
			pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, TOPSURFACE, Point, Normal);
			pHighlightVertexArray[iv++] = Point.x;
			pHighlightVertexArray[iv++] = Point.y;
			pHighlightVertexArray[iv++] = Point.z;
		}
		for (int lx=CHORDPOINTS-1; lx>=0; lx--)
		{
			double xRel = (double)lx/(double)(CHORDPOINTS-1);
			pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, BOTSURFACE, Point, Normal);
			pHighlightVertexArray[iv++] = Point.x;
			pHighlightVertexArray[iv++] = Point.y;
			pHighlightVertexArray[iv++] = Point.z;
		}
	}
	else
	{
		if((pWing->m_bSymetric || bRightSide) && !pWing->m_bIsFin)
		{
			m_nHighlightLines = 1;
			jSurf = pWing->m_Surface.size()/2 + iSection -1;

			for (int lx=0; lx<CHORDPOINTS; lx++)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, TOPSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}
			for (int lx=CHORDPOINTS-1; lx>=0; lx--)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, true, BOTSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}
		}

		if(pWing->m_bSymetric || !bRightSide)
		{
			m_nHighlightLines = 2;
			if(!pWing->m_bIsFin) jSurf = pWing->m_Surface.size()/2 - iSection;
			else                 jSurf = pWing->m_Surface.size()   - iSection;

			//plot A side outline
			for (int lx=0; lx<CHORDPOINTS; lx++)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, false, TOPSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}

			for (int lx=CHORDPOINTS-1; lx>=0; lx--)
			{
				double xRel = (double)lx/(double)(CHORDPOINTS-1);
				pWing->m_Surface.at(jSurf)->getSidePoint(xRel, false, BOTSURFACE, Point, Normal);
				pHighlightVertexArray[iv++] = Point.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z;
			}
		}
	}

	m_vboHighlight.destroy();
	m_vboHighlight.create();
	m_vboHighlight.bind();
	m_vboHighlight.allocate(pHighlightVertexArray, bufferSize*sizeof(float));
	m_vboHighlight.release();
	delete [] pHighlightVertexArray;
}


void GL3Widget::glMakeBodyFrameHighlight(Body *pBody, CVector bodyPos, int iFrame)
{
	int k;
	CVector Point;
	double hinc, u, v;
	if(iFrame<0) return;

	Frame *pFrame = pBody->frame(iFrame);
//	xinc = 0.1;
	hinc = 1.0/(double)(NH-1);

	m_nHighlightLines = 1;
	m_HighlightLineSize = NH * 2;
	int bufferSize = m_HighlightLineSize *2 *3 ;
	float *pHighlightVertexArray = new float[bufferSize];

	//create 3D Splines or Lines to overlay on the body
	int iv = 0;
	if(pBody->bodyType() == XFLR5::BODYSPLINETYPE)
	{
		if(pBody->activeFrame())
		{
			u = pBody->getu(pFrame->m_Position.x);
			v = 0.0;
			for (k=0; k<NH; k++)
			{
				pBody->getPoint(u,v,true, Point);
				pHighlightVertexArray[iv++] = Point.x+bodyPos.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z+bodyPos.z;
				v += hinc;
			}

			v = 1.0;
			for (k=0; k<NH; k++)
			{
				pBody->getPoint(u,v,false, Point);
				pHighlightVertexArray[iv++] = Point.x+bodyPos.x;
				pHighlightVertexArray[iv++] = Point.y;
				pHighlightVertexArray[iv++] = Point.z+bodyPos.z;
				v -= hinc;
			}
		}
	}
	else
	{
		for (k=0; k<pFrame->PointCount();k++)
		{
			pHighlightVertexArray[iv++] = pFrame->m_Position.x+bodyPos.x;
			pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].y;
			pHighlightVertexArray[iv++] = pFrame->m_CtrlPoint[k].z+bodyPos.z;
		}

		for (k=0; k<pFrame->PointCount();k++)
		{
			pHighlightVertexArray[iv++] =  pFrame->m_Position.x+bodyPos.x;
			pHighlightVertexArray[iv++] = -pFrame->m_CtrlPoint[k].y;
			pHighlightVertexArray[iv++] =  pFrame->m_CtrlPoint[k].z+bodyPos.z;
		}
	}
	m_vboHighlight.destroy();
	m_vboHighlight.create();
	m_vboHighlight.bind();
	m_vboHighlight.allocate(pHighlightVertexArray, bufferSize*sizeof(float));
	m_vboHighlight.release();
	delete [] pHighlightVertexArray;
}




