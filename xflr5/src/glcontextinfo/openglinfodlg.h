#ifndef OPENGLINFODLG_H
#define OPENGLINFODLG_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QSurface>
#include "MinTextEdit.h"

class OpenGLInfoDlg : public QDialog
{

public:
	OpenGLInfoDlg(QWidget *pParent=NULL);

private slots:
	void start();
	void renderWindowReady();
	void renderWindowError(const QString &msg);

private:
	void addVersions(QLayout *layout);
	void addProfiles(QLayout *layout);
	void addOptions(QLayout *layout);
	void addRenderableTypes(QLayout *layout);
	void addRenderWindow();
	void printFormat(const QSurfaceFormat &format);
	void setupLayout();

	QComboBox *m_version;
	QLayout *m_profiles;
	QLayout *m_options;
	QLayout *m_renderables;
	QTextEdit *m_glOutput;
	QTextEdit *m_extensions;
	QVBoxLayout *m_renderWindowLayout;
	QWidget *m_renderWindowContainer;
	QSurface *m_surface;
};

#endif // OPENGLINFODLG_H
