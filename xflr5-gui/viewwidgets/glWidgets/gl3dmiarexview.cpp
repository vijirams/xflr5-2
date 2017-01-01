#include <QOpenGLPaintDevice>
#include "gl3dmiarexview.h"
#include <miarex/Miarex.h>
#include <mainframe.h>
#include <QMenu>
#include <QApplication>
#include <QProgressDialog>
#include <Settings.h>
#include <globals.h>
#include <miarex/view/GL3DScales.h>
#include <objects3d/Surface.h>
#include <miarex/view/W3dPrefsDlg.h>


gl3dMiarexView::gl3dMiarexView(QWidget *parent) : gl3dView(parent)
{

}


void gl3dMiarexView::glRenderView()
{
	QMiarex* pMiarex = (QMiarex*)s_pMiarex;
	if(pMiarex->m_iView!=XFLR5::W3DVIEW) return;

	QMatrix4x4 modeMatrix;

	if(pMiarex->m_pCurWPolar && pMiarex->m_pCurWPolar->isStabilityPolar())
	{
		if(pMiarex->m_pCurPOpp && pMiarex->m_pCurPOpp->polarType()==XFLR5::STABILITYPOLAR)
		{
			QString strong = QString(tr("Time =")+"%1s").arg(pMiarex->m_ModeTime,6,'f',3);
			glRenderText(10, 15, strong);
		}

		modeMatrix.translate(pMiarex->m_ModeState[0], pMiarex->m_ModeState[1], pMiarex->m_ModeState[2]);
		modeMatrix.rotate(pMiarex->m_ModeState[3]*180.0/PI, 1.0, 0.0 ,0.0);
		modeMatrix.rotate(pMiarex->m_ModeState[4]*180.0/PI, 0.0, 1.0 ,0.0);
		modeMatrix.rotate(pMiarex->m_ModeState[5]*180.0/PI, 0.0, 0.0 ,1.0);
	}
	m_modelMatrix = modeMatrix;

	if(pMiarex->m_pCurPOpp)	m_modelMatrix.rotate(pMiarex->m_pCurPOpp->alpha(),0.0,1.0,0.0);
	m_pvmMatrix = m_OrthoMatrix * m_viewMatrix * m_modelMatrix;


	glEnable(GL_CLIP_PLANE0);

	if(pMiarex->m_pCurPlane)
	{
		m_modelMatrix = modeMatrix;

		// We use the model matrix to apply alpha and beta rotations to the geometry.
		if(pMiarex->m_pCurPOpp)
		{
			//apply aoa rotation
			m_modelMatrix.rotate(pMiarex->m_pCurPOpp->alpha(),0.0,1.0,0.0);

			/* CP position alredy includes the sideslip geometry, shond't be rotated by sideslip*/
			if(pMiarex->m_bXCP)
			{
				m_pvmMatrix = m_OrthoMatrix * m_viewMatrix * m_modelMatrix;
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintLift(iw);
				}
			}

			// apply sideslip
			if(fabs(pMiarex->m_pCurPOpp->beta())>PRECISION)
				m_modelMatrix.rotate(pMiarex->m_pCurPOpp->beta(), 0.0, 0.0, 1.0);
		}
		else
		{
			if(pMiarex->m_pCurWPolar && fabs(pMiarex->m_pCurWPolar->Beta())>0.001)
				m_modelMatrix.rotate(pMiarex->m_pCurWPolar->Beta(), 0.0, 0.0, 1.0);
		}
		m_pvmMatrix = m_OrthoMatrix * m_viewMatrix * m_modelMatrix;

		if(m_bVLMPanels)  paintMesh(pMiarex->matSize());
		if(pMiarex->m_pCurPOpp)
		{
			if(pMiarex->m_b3DCp && pMiarex->m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
			{
				paintPanelCp(pMiarex->matSize());
			}
			if(pMiarex->m_bPanelForce && pMiarex->m_pCurPOpp->analysisMethod()>=XFLR5::VLMMETHOD)
			{
				paintPanelForces(pMiarex->matSize());
			}
		}


		//streamlines and velocities are rotated by aoa when constructed
		if(pMiarex->m_pCurPOpp && pMiarex->m_bStream && pMiarex->m_pCurPOpp && !pMiarex->m_pCurPOpp->isLLTMethod() && !pMiarex->m_bResetglStream)
			paintStreamLines();

		if(pMiarex->m_pCurPOpp && pMiarex->m_bSurfVelocities && !pMiarex->m_pCurPOpp->isLLTMethod())
			paintSurfaceVelocities(pMiarex->matSize());

		m_ShaderProgramLine.bind();
		m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
		m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
		m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);
		m_ShaderProgramLine.release();

		if(m_bShowMasses) glDrawMasses(pMiarex->m_pCurPlane);


		for(int iw=0; iw<MAXWINGS; iw++)
		{
			Wing * pWing = pMiarex->m_pCurPlane->wing(iw);
			if(pWing)
			{
				paintWing(iw, pWing);
				if(m_bFoilNames) paintFoilNames(pWing);
			}
		}
		paintBody(pMiarex->m_pCurPlane->body());

		if(pMiarex->m_pCurPOpp)
		{
/*			if(pMiarex->m_bXCP)
			{
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintLift(iw);
				}
			}*/
			if(pMiarex->m_bMoments)
			{
				paintMoments();
			}
			if(pMiarex->m_bDownwash)
			{
				for(int iw=0; iw<MAXWINGS; iw++)
				{
					if(pMiarex->m_pCurPlane->wing(iw)) paintDownwash(iw);
				}
			}
			if(pMiarex->m_bICd || pMiarex->m_bVCd)
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
	glDisable(GL_CLIP_PLANE0);

}


void gl3dMiarexView::paintGL()
{
	QMiarex *pMiarex = (QMiarex*)s_pMiarex;
	pMiarex->glMake3DObjects();

	paintGL3();
	paintOverlay();
}


/**
*Overrides the contextMenuEvent method of the base class.
*Dispatches the handling to the active child application.
*/
void gl3dMiarexView::contextMenuEvent (QContextMenuEvent * event)
{
	MainFrame *pMainFrame = (MainFrame*)s_pMainFrame;
	QPoint ScreenPt = event->globalPos();
	m_bArcball = false;
	update();

	QMiarex *pMiarex = (QMiarex *)s_pMiarex;
	if (pMiarex->m_iView==XFLR5::W3DVIEW)
	{
		if(pMiarex->m_pCurWPolar && pMiarex->m_pCurWPolar->polarType()==XFLR5::STABILITYPOLAR)
			pMainFrame->m_pW3DStabCtxMenu->exec(ScreenPt);
		else pMainFrame->m_pW3DCtxMenu->exec(ScreenPt);
	}

}




void gl3dMiarexView::on3DReset()
{
	QMiarex *pMiarex = (QMiarex*)s_pMiarex;
	if(pMiarex->m_pCurPlane) startResetTimer(pMiarex->m_pCurPlane->span());
}



/**
*Overrides the resizeGL method of the base class.
* Sets the GL viewport to fit in the client area.
* Sets the scaling factors for the objects to be drawn in the viewport.
*@param width the width in pixels of the client area
*@param height the height in pixels of the client area
*/
void gl3dMiarexView::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);

	double w, h, s;
	w = (double)width;
	h = (double)height;
	s = 1.0;

	if(w>h)	m_GLViewRect.setRect(-s, s*h/w, s, -s*h/w);
	else    m_GLViewRect.setRect(-s*w/h, s, s*w/h, -s);

	m_PixTextOverlay = m_PixTextOverlay.scaled(rect().size()*devicePixelRatio());
	m_PixTextOverlay.fill(Qt::transparent);

	QMiarex* pMiarex = (QMiarex*)s_pMiarex;
	pMiarex->m_bResetTextLegend = true;
//	set3DScale();

}


void gl3dMiarexView::paintOverlay()
{
	QOpenGLPaintDevice device(size() * devicePixelRatio());
	QPainter painter(&device);

	QMiarex* pMiarex = (QMiarex*)s_pMiarex;
	if(pMiarex->m_bResetTextLegend) pMiarex->drawTextLegend();

	painter.drawPixmap(0,0, pMiarex->m_PixText);
	painter.drawPixmap(0,0, m_PixTextOverlay);
	m_PixTextOverlay.fill(Qt::transparent);
}

void gl3dMiarexView::glMakeCpLegendClr()
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

	int bufferSize = MAXCPCOLORS*2*6;
	float *pColorVertexArray = new float[bufferSize];

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
	Q_ASSERT(iv==bufferSize);

	m_vboLegendColor.destroy();
	m_vboLegendColor.create();
	m_vboLegendColor.bind();
	m_vboLegendColor.allocate(pColorVertexArray, colorLegendSize * sizeof(GLfloat));
	m_vboLegendColor.release();
	delete [] pColorVertexArray;
}



bool gl3dMiarexView::glMakeStreamLines(Wing *PlaneWing[MAXWINGS], CVector *pNode, WPolar *pWPolar, PlaneOpp *pPOpp, int nPanels)
{
	if(!pPOpp || !pWPolar || pWPolar->isLLTMethod()) return false;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	double memcoresize = Panel::coreSize();
	Panel::setCoreSize(0.0005); //mm, just for the time needed to build the streamlines which are very sensitive to trailing vortex interference

	QMiarex *pMiarex =(QMiarex*)s_pMiarex;

	Wing *pWing;

	QProgressDialog dlg(tr("Streamlines calculation"), tr("Abort"), 0, nPanels);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.show();

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
		if(PlaneWing[iw]) m_NStreamLines += m_Ny[iw]+20; //in case there is a body in the middle, other wise Ny+1
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

					//Tilt the geometry w.r.t. sideslip and aoa
					TC.rotateZ(RefPoint, pPOpp->beta());
					TD.rotateZ(RefPoint, pPOpp->beta());
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

						//Tilt the geometry w.r.t. sideslip and aoa
//						VA.rotateZ(pPOpp->beta());
//						VB.rotateZ(pPOpp->beta());
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
							pMiarex->m_thePanelAnalysis.getSpeedVector(C, Mu, Sigma, VT);

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
						pMiarex->m_theTask.m_pthePanelAnalysis->getSpeedVector(D, Mu, Sigma, VT);

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

				qApp->processEvents();
				if(dlg.wasCanceled()) break;
			}
			if(dlg.wasCanceled()) break;
		}
		if(dlg.wasCanceled()) break;
	}
//	if(!dlg.wasCanceled()) Q_ASSERT(iv==streamArraySize);

	m_NStreamLines = iv / GL3DScales::s_NX / 3;

//qDebug() << iv << streamArraySize;

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



void gl3dMiarexView::glMakeTransitions(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
{
	if(!pWing || !pWPolar || !pWOpp) return;
	int i,j,k,m;
	double yrel;
	CVector Pt, N;

	float *pTransVertexArray = NULL;
	int bufferSize = m_Ny[iWing]*6;
	pTransVertexArray = new float[bufferSize];
	int iv=0;
	if(pWPolar->isLLTMethod())
	{
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			pWing->surfacePoint(pWOpp->m_XTrTop[i], pWOpp->m_SpanPos[i], TOPSURFACE, Pt, N);

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
				for(k=0; k<pWing->m_Surface[j]->NYPanels(); k++)
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
				for(k=0; k<pWing->m_Surface[j]->NYPanels(); k++)
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

	if(pWPolar->isLLTMethod())
	{
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			pWing->surfacePoint(pWOpp->m_XTrBot[i], pWOpp->m_SpanPos[i], BOTSURFACE, Pt, N);
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
				for(k=0; k<pWing->m_Surface[j]->NYPanels(); k++)
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
				for(k=0; k<pWing->m_Surface[j]->NYPanels(); k++)
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

	Q_ASSERT(iv==m_Ny[iWing]*6);


	m_vboTransitions[iWing].destroy();
	m_vboTransitions[iWing].create();
	m_vboTransitions[iWing].bind();
	m_vboTransitions[iWing].allocate(pTransVertexArray, bufferSize * sizeof(GLfloat));
	m_vboTransitions[iWing].release();
	delete [] pTransVertexArray;
}



void gl3dMiarexView::glMakeSurfVelocities(Panel *pPanel, WPolar *pWPolar, PlaneOpp *pPOpp, int nPanels)
{
	if(!pWPolar || !pPOpp || pPOpp->isLLTMethod() || !pPanel)
		return;

	double factor;
	double length, sinT, cosT;

	double *Mu, *Sigma;
	double x1, x2, y1, y2, z1, z2, xe, ye, ze, dlx, dlz;
	CVector C, V, VT;
	CVector RefPoint(0.0,0.0,0.0);

	factor = QMiarex::s_VelocityScale/100.0;

	QMiarex *pMiarex = (QMiarex*)s_pMiarex;


	QProgressDialog dlg(tr("Velocities calculation"), tr("Abort"), 0, nPanels);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.show();


	Mu    = pPOpp->m_dG;
	Sigma = pPOpp->m_dSigma;

	// vertices array size:
	//		nPanels x 1 arrow
	//      x3 lines per arrow
	//      x2 vertices per line
	//		x3 = 3 vertex components

	int velocityVertexSize = nPanels * 3 * 2 * 3;
	float *velocityVertexArray = new float[velocityVertexSize];

	int iv=0;
	for (int p=0; p<nPanels; p++)
	{
		VT.set(pPOpp->m_QInf,0.0,0.0);

		if(pWPolar->analysisMethod()==XFLR5::PANELMETHOD)
		{
			if(pPanel[p].m_Pos==MIDSURFACE) C.copy(pPanel[p].CtrlPt);
			else                            C.copy(pPanel[p].CollPt);
			pMiarex->m_theTask.m_pthePanelAnalysis->getSpeedVector(C, Mu, Sigma, V);

			VT += V;

			//Tilt the geometry w.r.t. sideslip and aoa
			C.rotateZ(RefPoint, pPOpp->beta());
			C.rotateY(RefPoint, pPOpp->alpha());

		}
		length = VT.VAbs()*factor;
		xe     = C.x+factor*VT.x;
		ye     = C.y+factor*VT.y;
		ze     = C.z+factor*VT.z;
		if(length>0.0)
		{
			cosT   = (xe-C.x)/length;
			sinT   = (ze-C.z)/length;
			dlx    = 0.15*length;
			dlz    = 0.07*length;
		}
		else
		{
			cosT   = 0.0;
			sinT   = 0.0;
			dlx    = 0.0;
			dlz    = 0.0;
		}

		x1 = xe -dlx*cosT - dlz*sinT;
		y1 = ye;
		z1 = ze -dlx*sinT + dlz*cosT;

		x2 = xe -dlx*cosT + dlz*sinT;
		y2 = ye;
		z2 = ze -dlx*sinT - dlz*cosT;

		velocityVertexArray[iv++] = C.x;
		velocityVertexArray[iv++] = C.y;
		velocityVertexArray[iv++] = C.z;
		velocityVertexArray[iv++] = xe;
		velocityVertexArray[iv++] = ye;
		velocityVertexArray[iv++] = ze;

		velocityVertexArray[iv++] = xe;
		velocityVertexArray[iv++] = ye;
		velocityVertexArray[iv++] = ze;
		velocityVertexArray[iv++] = x1;
		velocityVertexArray[iv++] = y1;
		velocityVertexArray[iv++] = z1;

		velocityVertexArray[iv++] = xe;
		velocityVertexArray[iv++] = ye;
		velocityVertexArray[iv++] = ze;
		velocityVertexArray[iv++] = x2;
		velocityVertexArray[iv++] = y2;
		velocityVertexArray[iv++] = z2;
		dlg.setValue(p);
		qApp->processEvents();
		if(dlg.wasCanceled()) break;
	}

	if(!dlg.wasCanceled()) Q_ASSERT(iv==velocityVertexSize);

	m_vboSurfaceVelocities.destroy();
	m_vboSurfaceVelocities.create();
	m_vboSurfaceVelocities.bind();
	m_vboSurfaceVelocities.allocate(velocityVertexArray, velocityVertexSize * sizeof(GLfloat));
	m_vboSurfaceVelocities.release();

	delete [] velocityVertexArray;
}


void gl3dMiarexView::paintDownwash(int iWing)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_DownwashColor);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

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

void gl3dMiarexView::glMakeLiftForce(WPolar *pWPolar, PlaneOpp *pPOpp)
{
	if(!pWPolar || !pPOpp) return;

	double forcez,forcex,glx, gly, glz;
	double sign;

	double force = 0.5*pWPolar->density() * pWPolar->referenceArea()
	*pPOpp->m_QInf*pPOpp->m_QInf
	*pPOpp->m_CL;

	force *= QMiarex::s_LiftScale/500.0;

	forcez =  force * cos(pPOpp->alpha() * PI/180.0);
	forcex = -force * sin(pPOpp->alpha() * PI/180.0);

	if (force>0.0) sign = 1.0; else sign = -1.0;

	glx = (GLfloat)pPOpp->m_CP.x;
	gly = (GLfloat)pPOpp->m_CP.y;
	glz = (GLfloat)pPOpp->m_CP.z;

	float *liftForceVertexArray = new float[18];
	int iv=0;
	liftForceVertexArray[iv++] = glx;
	liftForceVertexArray[iv++] = gly;
	liftForceVertexArray[iv++] = glz;
	liftForceVertexArray[iv++] = glx+forcex;
	liftForceVertexArray[iv++] = gly;
	liftForceVertexArray[iv++] = glz+forcez;


	liftForceVertexArray[iv++] = glx+forcex;
	liftForceVertexArray[iv++] = gly;
	liftForceVertexArray[iv++] = glz+forcez;
	liftForceVertexArray[iv++] = glx+forcex+0.008;
	liftForceVertexArray[iv++] = gly+0.008;
	liftForceVertexArray[iv++] = glz+forcez-0.012*sign;

	liftForceVertexArray[iv++] = glx+forcex;
	liftForceVertexArray[iv++] = gly;
	liftForceVertexArray[iv++] = glz+forcez;
	liftForceVertexArray[iv++] = glx+forcex-0.008;
	liftForceVertexArray[iv++] = gly-0.008;
	liftForceVertexArray[iv++] = glz+forcez-0.012*sign;

	m_vboLiftForce.destroy();
	m_vboLiftForce.create();
	m_vboLiftForce.bind();
	m_vboLiftForce.allocate(liftForceVertexArray, 18*sizeof(float));
	m_vboLiftForce.release();
	delete [] liftForceVertexArray;

}

void gl3dMiarexView::glMakeMoments(Wing *pWing, WPolar *pWPolar, PlaneOpp *pPOpp)
{
//	The most common aeronautical convention defines
//	- the roll as acting about the longitudinal axis, positive with the starboard wing down.
//	- The yaw is about the vertical body axis, positive with the nose to starboard.
//	- Pitch is about an axis perpendicular to the longitudinal plane of symmetry, positive nose up.
//	-- Wikipedia flight dynamics --
	if(!pWing || !pWPolar) return;

	int i;

	double ampL, ampM, ampN;
	double sign;
	double angle=0.0;//radian
	double endx, endy, endz, dx, dy, dz,xae, yae, zae;
	double factor = 10.0;
	double radius= pWing->m_PlanformSpan/4.0;

	m_iMomentPoints = 0;

	ampL = 0.5*pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceChordLength()
			*pPOpp->m_QInf*pPOpp->m_QInf * pPOpp->m_GRm * QMiarex::s_LiftScale*factor;
	ampM = 0.5*pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceSpanLength()
			*pPOpp->m_QInf*pPOpp->m_QInf * pPOpp->m_GCm * QMiarex::s_LiftScale*factor;
	ampN = 0.5*pWPolar->density() * pWPolar->referenceArea() * pWPolar->referenceSpanLength()
			*pPOpp->m_QInf*pPOpp->m_QInf*(pPOpp->m_GYm) * QMiarex::s_LiftScale*factor;

	if(fabs(ampL)>PRECISION)
	{
		m_iMomentPoints += int(qAbs(ampL))  *2;
		m_iMomentPoints += 4;
	}
	if(fabs(ampM)>PRECISION)
	{
		m_iMomentPoints += int(qAbs(ampM))  *2;
		m_iMomentPoints += 4;
	}
	if(fabs(ampN)>PRECISION)
	{
		m_iMomentPoints += int(qAbs(ampN))  *2;
		m_iMomentPoints += 4;
	}

	float *momentVertexArray = new float[m_iMomentPoints*3];
	int iv = 0;

	//ROLLING MOMENT
	if(fabs(ampL)>PRECISION)
	{
		if (ampL>0.0) sign = -1.0; else sign = 1.0;
		for (i=0; i<int(qAbs(ampL)); i++)
		{
			angle = sign*(double)i*PI/180.0     / factor;
			momentVertexArray[iv++] = 0.0;
			momentVertexArray[iv++] = radius*cos(angle);
			momentVertexArray[iv++] = radius*sin(angle);
			angle = sign*(double)(i+1)*PI/180.0 / factor;
			momentVertexArray[iv++] = 0.0;
			momentVertexArray[iv++] = radius*cos(angle);
			momentVertexArray[iv++] = radius*sin(angle);
		}

		endy = radius*cos(angle);
		endz = radius*sin(angle);

		dy = 0.03;
		dz = 0.03*sign;

		yae = (radius-dy)*cos(angle) +dz *sin(angle);
		zae = (radius-dy)*sin(angle) -dz *cos(angle);
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = endy;
		momentVertexArray[iv++] = endz;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = yae;
		momentVertexArray[iv++] = zae;

		yae = (radius+dy)*cos(angle) +dz *sin(angle);
		zae = (radius+dy)*sin(angle) -dz *cos(angle);
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = endy;
		momentVertexArray[iv++] = endz;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = yae;
		momentVertexArray[iv++] = zae;
	}

	//PITCHING MOMENT
	if(fabs(ampM)>PRECISION)
	{
		if (ampM>0.0) sign = -1.0; else sign = 1.0;
		for (i=0; i<int(qAbs(ampM)); i++)
		{
			angle = sign*(double)i*PI/180.0     / factor;
			momentVertexArray[iv++] = radius*cos(angle);
			momentVertexArray[iv++] = 0.0;
			momentVertexArray[iv++] = radius*sin(angle);
			angle = sign*(double)(i+1)*PI/180.0 / factor;
			momentVertexArray[iv++] = radius*cos(angle);
			momentVertexArray[iv++] = 0.0;
			momentVertexArray[iv++] = radius*sin(angle);
		}
		endx = radius*cos(angle);
		endz = radius*sin(angle);

		dx = 0.03;
		dz = 0.03*sign;

		xae = (radius-dx)*cos(angle) +dz *sin(angle);
		zae = (radius-dx)*sin(angle) -dz *cos(angle);
		momentVertexArray[iv++] = endx;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = endz;
		momentVertexArray[iv++] = xae;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = zae;

		xae = (radius+dx)*cos(angle) +dz *sin(angle);
		zae = (radius+dx)*sin(angle) -dz *cos(angle);
		momentVertexArray[iv++] = endx;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = endz;
		momentVertexArray[iv++] = xae;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = zae;
	}


	//YAWING MOMENT
	if(fabs(ampN)>PRECISION)
	{
		if (ampN>0.0) sign = -1.0; else sign = 1.0;
		angle = 0.0;
		for (i=0; i<int(qAbs(ampN)); i++)
		{
			angle = sign*(double)i*PI/180.0     / factor;
			momentVertexArray[iv++] = -radius*cos(angle);
			momentVertexArray[iv++] = -radius*sin(angle);
			momentVertexArray[iv++] = 0.0;
			angle = sign*(double)(i+1)*PI/180.0 / factor;
			momentVertexArray[iv++] = -radius*cos(angle);
			momentVertexArray[iv++] = -radius*sin(angle);
			momentVertexArray[iv++] = 0.0;
		}

		endx = -radius*cos(angle);
		endy = -radius*sin(angle);

		dx =   0.03;
		dy =  -0.03*sign;

		xae = (-radius+dx)*cos(angle) +dy *sin(angle);
		yae = (-radius+dx)*sin(angle) -dy *cos(angle);
		momentVertexArray[iv++] = endx;
		momentVertexArray[iv++] = endy;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = xae;
		momentVertexArray[iv++] = yae;
		momentVertexArray[iv++] = 0.0;

		xae = (-radius-dx)*cos(angle) +dy *sin(angle);
		yae = (-radius-dx)*sin(angle) -dy *cos(angle);
		momentVertexArray[iv++] = endx;
		momentVertexArray[iv++] = endy;
		momentVertexArray[iv++] = 0.0;
		momentVertexArray[iv++] = xae;
		momentVertexArray[iv++] = yae;
		momentVertexArray[iv++] = 0.0;
	}


	Q_ASSERT(iv==m_iMomentPoints*3);
	m_vboMoments.destroy();
	m_vboMoments.create();
	m_vboMoments.bind();
	m_vboMoments.allocate(momentVertexArray, m_iMomentPoints*3*sizeof(float));
	m_vboMoments.release();
	delete [] momentVertexArray;
}

void gl3dMiarexView::glMakeLiftStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
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

	int iv;
	if(pWPolar->isLLTMethod())
	{
		iv=0;
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
			pWing->surfacePoint(pWOpp->m_XCPSpanRel[i], pWOpp->m_SpanPos[i], MIDSURFACE, Pt, PtNormal);

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
			pWing->surfacePoint(pWOpp->m_XCPSpanRel[i], pWOpp->m_SpanPos[i], MIDSURFACE, Pt, PtNormal);

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
		iv=0;
		//lift lines
		for (j=0; j<pWing->m_Surface.size(); j++)
		{
			for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
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

			for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
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
	Q_ASSERT(iv==m_Ny[iWing]*9);

	m_vboLiftStrips[iWing].destroy();
	m_vboLiftStrips[iWing].create();
	m_vboLiftStrips[iWing].bind();
	m_vboLiftStrips[iWing].allocate(pLiftVertexArray, m_Ny[iWing]*9 * sizeof(GLfloat));
	m_vboLiftStrips[iWing].release();
	delete [] pLiftVertexArray;
}

void gl3dMiarexView::paintLift(int iWing)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_XCPColor);
	m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, m_modelMatrix);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

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

	m_vboLiftStrips[iWing].bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
	glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
	glDrawArrays(GL_LINE_STRIP, m_Ny[iWing]*2, m_Ny[iWing]);
	m_vboLiftStrips[iWing].release();

	m_vboLiftForce.bind();
	glLineWidth((GLfloat)W3dPrefsDlg::s_XCPWidth*2.0);
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);
	glDrawArrays(GL_LINES, 0, 6);
	m_vboLiftForce.release();

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}

void gl3dMiarexView::paintMoments()
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_MomentColor);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

	m_vboMoments.bind();
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_LINE_STIPPLE);

	switch(W3dPrefsDlg::s_MomentStyle)
	{
		case 1: glLineStipple (1, 0xCFCF); break;
		case 2: glLineStipple (1, 0x6666); break;
		case 3: glLineStipple (1, 0xFF18); break;
		case 4: glLineStipple (1, 0x7E66); break;
		default: glLineStipple (1, 0xFFFF); break;
	}

	glLineWidth((GLfloat)W3dPrefsDlg::s_MomentWidth);

	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3);

	glDrawArrays(GL_LINES, 0, m_iMomentPoints);
	m_vboMoments.release();

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}

void gl3dMiarexView::glMakeDownwash(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp)
{
	if(!pWing || !pWPolar || !pWOpp) return;

	int i,j,k,p;
	double dih, yob;
	double y1, y2, z1, z2, xs, ys, zs;
	CVector C, Pt, PtNormal;
	double factor, amp;

	double sina = -sin(pWOpp->m_Alpha*PI/180.0);
	double cosa =  cos(pWOpp->m_Alpha*PI/180.0);
	factor = QMiarex::s_VelocityScale/5.0;

	int bufferSize = m_Ny[iWing]*18;
	float *pDownWashVertexArray = new float[bufferSize];
	int iv = 0;
	if(pWPolar->isLLTMethod())
	{
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;
			pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], MIDSURFACE, Pt, PtNormal);

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
			for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
			{
//				m_pSurface[j+surf0]->GetTrailingPt(k, C);
				pWing->m_Surface[j]->getTrailingPt(k, C);
//				if (pWOpp->m_Vd[i].z>0) sign = 1.0; else sign = -1.0;
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

	Q_ASSERT(iv==bufferSize);

	m_vboDownwash[iWing].destroy();
	m_vboDownwash[iWing].create();
	m_vboDownwash[iWing].bind();
	m_vboDownwash[iWing].allocate(pDownWashVertexArray, bufferSize * sizeof(GLfloat));
	m_vboDownwash[iWing].release();
	delete [] pDownWashVertexArray;
}

void gl3dMiarexView::glMakeDragStrip(int iWing, Wing *pWing, WPolar *pWPolar, WingOpp *pWOpp, double beta)
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
	cosb =  cos(-beta*PI/180.0);
	sinb =  sin(-beta*PI/180.0);

	int bufferSize = m_Ny[iWing]*9;
	pICdVertexArray = new float[bufferSize];
	pVCdVertexArray = new float[bufferSize];

	//DRAGLINE
	double q0 = 0.5 * pWPolar->density() * pWPolar->referenceArea() * pWOpp->m_QInf * pWOpp->m_QInf;

	int ii, iv;
	if(pWPolar->isLLTMethod())
	{
		ii=0;
		iv=0;
		for (i=1; i<pWOpp->m_NStation; i++)
		{
			yob = 2.0*pWOpp->m_SpanPos[i]/pWOpp->m_Span;

			pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], MIDSURFACE, Pt, PtNormal);
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
				pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], MIDSURFACE, Pt, PtNormal);

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
				pWing->surfacePoint(1.0, pWOpp->m_SpanPos[i], MIDSURFACE, Pt, PtNormal);

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
		ii=0;
		iv=0;
		for (j=0; j<pWing->m_Surface.size(); j++)
		{
			//All surfaces
			for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
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
					for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
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
					for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
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
					for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
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
					for (k=0; k< pWing->m_Surface[j]->NYPanels(); k++)
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
	if(pMiarex->m_bICd) Q_ASSERT(ii==bufferSize);
	if(pMiarex->m_bVCd) Q_ASSERT(iv==bufferSize);


	m_vboICd[iWing].destroy();
	m_vboICd[iWing].create();
	m_vboICd[iWing].bind();
	m_vboICd[iWing].allocate(pICdVertexArray, bufferSize * sizeof(GLfloat));
	m_vboICd[iWing].release();
	delete [] pICdVertexArray;

	m_vboVCd[iWing].destroy();
	m_vboVCd[iWing].create();
	m_vboVCd[iWing].bind();
	m_vboVCd[iWing].allocate(pVCdVertexArray, bufferSize * sizeof(GLfloat));
	m_vboVCd[iWing].release();
	delete [] pVCdVertexArray;
}

void gl3dMiarexView::paintDrag(int iWing)
{
	QMiarex *pMiarex= (QMiarex*)s_pMiarex;

	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

	glEnable(GL_DEPTH_TEST);
	glEnable (GL_LINE_STIPPLE);

	// Induced drag
	if(pMiarex->m_bICd)
	{
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


		glDrawArrays(GL_LINES, 0, m_Ny[iWing]*2);
		glDrawArrays(GL_LINE_STRIP, m_Ny[iWing]*2, m_Ny[iWing]);

		m_vboICd[iWing].release();
	}

	//Viscous drag
	if(pMiarex->m_bVCd)
	{
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
	}

	glDisable (GL_LINE_STIPPLE);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.release();
}

void gl3dMiarexView::paintStreamLines()
{
	QMatrix4x4 idMatrix;
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.setUniformValue(m_mMatrixLocationLine, idMatrix);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_OrthoMatrix * m_viewMatrix);

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

void gl3dMiarexView::paintTransitions(int iWing)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_vMatrixLocationLine, m_viewMatrix);
	m_ShaderProgramLine.setUniformValue(m_pvmMatrixLocationLine, m_pvmMatrix);

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

void gl3dMiarexView::paintSurfaceVelocities(int nPanels)
{
	m_ShaderProgramLine.bind();
	m_ShaderProgramLine.enableAttributeArray(m_VertexLocationLine);
	m_ShaderProgramLine.setUniformValue(m_ColorLocationLine, W3dPrefsDlg::s_WakeColor);
	m_vboSurfaceVelocities.bind();
	m_ShaderProgramLine.setAttributeBuffer(m_VertexLocationLine, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	glLineWidth(W3dPrefsDlg::s_WakeWidth);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_LINES, 0, nPanels*3*2);

	m_ShaderProgramLine.disableAttributeArray(m_ColorLocationLine);
	m_ShaderProgramLine.disableAttributeArray(m_VertexLocationLine);
	m_vboSurfaceVelocities.release();
	m_ShaderProgramLine.release();
}

void gl3dMiarexView::paintCpLegendClr()
{
	m_ShaderProgramGradient.bind();
	m_ShaderProgramGradient.enableAttributeArray(m_VertexLocationGradient);
	m_ShaderProgramGradient.enableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.setUniformValue(m_pvmMatrixLocationGradient, m_OrthoMatrix);
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

void gl3dMiarexView::paintPanelCp(int nPanels)
{
	m_ShaderProgramGradient.bind();
	m_ShaderProgramGradient.enableAttributeArray(m_VertexLocationGradient);
	m_ShaderProgramGradient.enableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.setUniformValue(m_pvmMatrixLocationGradient, m_pvmMatrix);
	m_vboPanelCp.bind();
	m_ShaderProgramGradient.setAttributeBuffer(m_VertexLocationGradient, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
	m_ShaderProgramGradient.setAttributeBuffer(m_ColorLocationGradient,  GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glDrawArrays(GL_TRIANGLES, 0, nPanels*2*3);
	glDisable(GL_POLYGON_OFFSET_FILL);

	m_ShaderProgramGradient.disableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.disableAttributeArray(m_VertexLocationGradient);
	m_vboPanelCp.release();
	m_ShaderProgramGradient.release();
}

void gl3dMiarexView::paintPanelForces(int nPanels)
{
	m_ShaderProgramGradient.bind();
	m_ShaderProgramGradient.enableAttributeArray(m_VertexLocationGradient);
	m_ShaderProgramGradient.enableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.setUniformValue(m_pvmMatrixLocationGradient, m_pvmMatrix);
	m_vboPanelForces.bind();
	m_ShaderProgramGradient.setAttributeBuffer(m_VertexLocationGradient, GL_FLOAT, 0,                  3, 6 * sizeof(GLfloat));
	m_ShaderProgramGradient.setAttributeBuffer(m_ColorLocationGradient,  GL_FLOAT, 3* sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	glLineWidth(W3dPrefsDlg::s_XCPWidth);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_LINES, 0, nPanels*3*2);

	m_ShaderProgramGradient.disableAttributeArray(m_ColorLocationGradient);
	m_ShaderProgramGradient.disableAttributeArray(m_VertexLocationGradient);
	m_vboPanelForces.release();
	m_ShaderProgramGradient.release();
}



void gl3dMiarexView::set3DRotationCenter(QPoint point)
{
	//adjusts the new rotation center after the user has picked a point on the screen
	//finds the closest panel under the point,
	//and changes the rotation vector and viewport translation
	CVector I, A, B, AA, BB, PP;

	screenToViewport(point, B);
	B.z = -1.0;
	A.set(B.x, B.y, +1.0);

	viewportToWorld(A, AA);
	viewportToWorld(B, BB);

	m_transIncrement.set(BB.x-AA.x, BB.y-AA.y, BB.z-AA.z);
	m_transIncrement.normalize();

	bool bIntersect = false;

	QMiarex *pMiarex = (QMiarex*)s_pMiarex;
	if(pMiarex->intersectObject(AA, m_transIncrement, I))
	{
		bIntersect = true;
		PP.set(I);
	}


	if(bIntersect)
	{
		startTranslationTimer(PP);
	}
}

