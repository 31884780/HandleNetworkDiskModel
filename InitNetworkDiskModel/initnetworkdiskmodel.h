#pragma once
#pragma execution_character_set("utf-8")
/*
	初始化设置硬盘模块。修改ip，设置启动参数
	需要的命令列表模板放在软件目录下
	运行时需要ip配置表文件
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
			QString imgPath;		//纯路径，不带文件名
			QString username;
			QString password;
			QString imgName;		//纯文件名，不含路径

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
		 QSharedPointer<SshShell> m_shell;			//ssh命令执行类
		QVector<modelSetInfo> m_modelInfoList;		//模块配置文件中读出的待设置模块配置信息
		int m_currentModelInfoIndex;				//当前配置文件行索引
		QString m_modelInfoFile;					//模块配置文件 
		QString m_doneFile;							//已完成配置记录
	};
}
