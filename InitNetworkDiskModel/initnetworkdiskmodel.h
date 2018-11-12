#pragma once
#pragma execution_character_set("utf-8")
/*
	��ʼ������Ӳ��ģ�顣�޸�ip��������������
	��Ҫ�������б�ģ��������Ŀ¼��
	����ʱ��Ҫip���ñ��ļ�
*/

#include "ui_initnetworkdiskmodel.h"

#include <QtWidgets/QWidget>
#include <QStringList>
#include <QSharedPointer>

#include "ssh\sshconnection.h"

namespace DiskModel {
	class SshShell;
	class InitNetworkDiskModel : public QWidget
	{
		Q_OBJECT

	public:
		InitNetworkDiskModel(QWidget *parent = Q_NULLPTR);
		~InitNetworkDiskModel();

	private slots:
		void handleExecuteButtonPush();
		void handleOpenCfgFile();
		void handleNextCmd();
		void handleMountChange();

// 		void handleSshStatus(int status);
// 		void handleSshStdErr(QString err);
// 		void handleSshStdOut(QString out);
// 		void handleSshMessage(QString msg);

		void handleSshDisconnected();
		void handleSshMessage(QString sshMsg);
		void handleSshErr(QSsh::SshError err);

	private:
		QStringList generateCmdListFromCmdView();
		QSsh::SshConnectionParameters generateConnParameters();
		bool getModelSetInfoFromFile(QString fileName);
		void setCurrentCmdListToEdit(int idx);

		SshShell* newShellRunner(QSsh::SshConnectionParameters parm, QStringList cmdList);

		void enableButton(bool b);
	private:
		struct modelSetInfo
		{
			QString ip;
			QString mac;
			QString mask;
			QString imgPath;		//��·���������ļ���
			QString username;
			QString password;
			QString imgName;		//���ļ���������·��

			bool isAllEmpty();
		};
		enum colName
		{
			COL_IP,
			COL_MAC,
			COL_IMG,
			COL_USER,
			COL_PSW,
			TOTALCOLS,
		};

	private:
		Ui::initnetworkdiskmodel ui;
		 QSharedPointer<SshShell> m_shell;			//ssh����ִ����
		QVector<modelSetInfo> m_modelInfoList;		//ģ�������ļ��ж����Ĵ�����ģ��������Ϣ
		int m_currentModelInfoIndex;				//��ǰ�����ļ�������
		QString m_modelInfoFile;					//ģ�������ļ� 
		QString m_doneFile;							//��������ü�¼
	};
}
