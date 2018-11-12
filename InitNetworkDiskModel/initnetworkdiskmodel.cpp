#include "initnetworkdiskmodel.h"
#include "ssh\sshconnection.h"
#include "sshshell.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

using namespace QSsh;
namespace DiskModel {
	InitNetworkDiskModel::InitNetworkDiskModel(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
		setWindowTitle(QString("模块批量配置").toUtf8());
		//
		connect(ui.pushButton_open, SIGNAL(clicked()), this, SLOT(handleOpenCfgFile()));
		connect(ui.pushButton_exe, SIGNAL(clicked()), this, SLOT(handleExecuteButtonPush()));
		connect(ui.pushButton_next, SIGNAL(clicked()), this, SLOT(handleNextCmd()));
		connect(ui.checkBox_doubleimg, SIGNAL(stateChanged(int)), this, SLOT(handleMountChange()));
		connect(ui.checkBox_mounttest, SIGNAL(stateChanged(int)), this, SLOT(handleMountChange()));
		//
		m_shell.reset(NULL);
		//
		ui.pushButton_exe->setEnabled(false);

		m_currentModelInfoIndex = 0;
	}

	InitNetworkDiskModel::~InitNetworkDiskModel()
	{

	}

	void InitNetworkDiskModel::handleExecuteButtonPush()
	{
		//准备
		if (ui.lineEdit_cfgfile->text().isEmpty())
		{
			QMessageBox::warning(this, "警告！", "请设置配置文件！", QMessageBox::Yes);
			return;
		}
		if (m_modelInfoList.isEmpty())
		{
			QMessageBox::warning(this, "警告！", "参数文件为空！", QMessageBox::Yes);
			return;
		}
		QStringList cmdList = generateCmdListFromCmdView();
		if (cmdList.isEmpty())
		{
			QMessageBox::warning(this, "警告！", "命令列表为空！", QMessageBox::Yes);
			return;
		}

		//连接、执行命令
 		SshConnectionParameters parm = generateConnParameters();
		QStringList shellList = generateCmdListFromCmdView();
		m_shell.reset(newShellRunner(parm, shellList));

		//
		enableButton(false);
	}

	void InitNetworkDiskModel::handleOpenCfgFile()
	{
		QString fileName = QFileDialog::getOpenFileName(this, tr("打开配置文件"),
			QCoreApplication::applicationDirPath(),
			"*.csv (*.csv);;TEXT (*.txt);;*.* (*.*)");
		if (fileName.isEmpty())
		{
			return;
		}

		ui.lineEdit_cfgfile->setText(fileName);
		m_modelInfoFile = fileName;
		QFileInfo info(fileName);
		m_doneFile = QString("%1/ip_done.txt").arg(info.absolutePath());

		getModelSetInfoFromFile(fileName);
	}

	void InitNetworkDiskModel::handleNextCmd()
	{
		//配置文件中删除最近设置过的一行,已完成配置文件增加一行
		if (!m_modelInfoFile.isEmpty() && m_currentModelInfoIndex < m_modelInfoList.count())
		{
			QFile file(m_modelInfoFile);
			if (!file.open(QIODevice::Text | QIODevice::ReadOnly))
			{
				return;
			}
			QString textContent = file.readAll();
			int endOfFirstLine = textContent.indexOf("\n");
			QString remainContent, doneRecord;

			if (endOfFirstLine < 0)		//最后一行
			{
				remainContent = "";
				doneRecord = QString("%1\n").arg(textContent);
			}
			else
			{
				remainContent = textContent.mid(endOfFirstLine + 1);		//删除后内容
				doneRecord = QString("%1\n").arg(textContent.left(endOfFirstLine));				//删除的行

			}
			file.close();

			if (!file.open(QIODevice::Text | QIODevice::Truncate | QIODevice::WriteOnly))
			{
				return;
			}
			file.write(remainContent.toStdString().c_str());
			file.close();

			//保存已完成记录

			QFile doneFile(m_doneFile);
			if (!doneFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append))
			{
				return;
			}
			doneFile.write(doneRecord.toStdString().c_str());
		}

		//执行结束后计数器加1	
		++m_currentModelInfoIndex;
		if (m_currentModelInfoIndex >= m_modelInfoList.count())
		{
			QMessageBox::warning(this, "警告！", "已经是最后一行配置！", QMessageBox::Yes);
			return;
		}

		setCurrentCmdListToEdit(m_currentModelInfoIndex);
		ui.textBrowser_result->clear();
	}

	void InitNetworkDiskModel::handleSshDisconnected()
	{
		QString msg("*************命令执行结束*************");
		ui.textBrowser_result->append(msg);

		//打开按钮
		enableButton(true);
	}


	void InitNetworkDiskModel::handleSshMessage(QString sshMsg)
	{
		sshMsg = sshMsg.trimmed();
		if (!sshMsg.isEmpty())
		{
			ui.textBrowser_result->append(sshMsg);
		}
	}

	void InitNetworkDiskModel::handleSshErr(QSsh::SshError err)
	{
		QString info;
		switch (err)
		{
		case QSsh::SshNoError:
			break;
		case QSsh::SshSocketError:
			info = "SshSocketError";
			break;
		case QSsh::SshTimeoutError:
			info = "SshTimeoutError";
			break;
		case QSsh::SshProtocolError:
			info = "SshProtocolError";
			break;
		case QSsh::SshHostKeyError:
			info = "SshHostKeyError";
			break;
		case QSsh::SshKeyFileError:
			info = "SshKeyFileError";
			break;
		case QSsh::SshAuthenticationError:
			info = "SshAuthenticationError";
			break;
		case QSsh::SshClosedByServerError:
			info = "SshClosedByServerError";
			break;
		case QSsh::SshInternalError:
			info = "SshInternalError";
			break;
		default:
			break;
		}
		if (!info.isEmpty())
		{
			QString err = QString("===>connect error: %1").arg(info);
			ui.textBrowser_result->append(err);
		}
	}

	QStringList InitNetworkDiskModel::generateCmdListFromCmdView()
	{
		//改成从对话框中取，留一个修改的机会
		QStringList cmdList;
		QString totalCmd = ui.textEdit_cmd->toPlainText();
		cmdList = totalCmd.split("\n", QString::SkipEmptyParts);

		return cmdList;
	}

	SshConnectionParameters InitNetworkDiskModel::generateConnParameters()
	{
		QString host = ui.lineEdit_conip->text();
		QString userName = "root";
		QString password = "admin";
		quint16 port = 22;


		SshConnectionParameters parm;
		parm.host = host;
		parm.userName = userName;
		parm.password = password;
		parm.port = port;
		parm.timeout = 1000;
		parm.authenticationType = SshConnectionParameters::AuthenticationTypePassword;
		parm.hostKeyCheckingMode = QSsh::SshHostKeyCheckingNone;

		return parm;
	}

	bool InitNetworkDiskModel::getModelSetInfoFromFile(QString fileName)
	{
		QFile file(fileName);
		m_modelInfoList.clear();

		//读内容 (ip,mac,带文件名imgPath,username,password)
		if (!file.open(QIODevice::Text | QIODevice::ReadWrite))
		{
			QMessageBox::warning(this, "警告！", "配置文件打开错误！", QMessageBox::Yes);
			return false;
		}
		while (!file.atEnd())
		{
			QString line = file.readLine(500);
			QStringList strList = line.split(",");
			if (strList.isEmpty() || strList.count() != TOTALCOLS)	//执行命令时同步处理配置文件，成功的移入成功列表文件，处理不了的直接删除
			{
				continue;
			}

			modelSetInfo modelInfo;
			modelInfo.ip = strList[COL_IP].trimmed();
			modelInfo.mac = strList[COL_MAC].trimmed();
			modelInfo.username = strList[COL_USER].trimmed();
			modelInfo.password = strList[COL_PSW].trimmed();
			modelInfo.mask = ui.lineEdit_mask->text().trimmed();

			QString fullPath = strList[COL_IMG].trimmed();
			int idx = fullPath.lastIndexOf("/");
			modelInfo.imgPath = fullPath.left(idx);
			modelInfo.imgName = fullPath.right(fullPath.count() - idx - 1);

			m_modelInfoList.push_back(modelInfo);
		}

		setCurrentCmdListToEdit(0);
		ui.pushButton_exe->setEnabled(true);
		file.close();

		return true;
	}

	void InitNetworkDiskModel::setCurrentCmdListToEdit(int idx)
	{
		if (m_modelInfoList.isEmpty())
		{
			return;
		}
		ui.textEdit_cmd->clear();
		bool mountTest = ui.checkBox_mounttest->isChecked();
		bool doubleImg = ui.checkBox_doubleimg->isChecked();

		QString totalCmd;
		modelSetInfo currentSetInfo = m_modelInfoList[idx];

		//挂载测试
		if (mountTest)
		{
			QString dd = "dd if=/dev/zero of=/tmp/a.img bs=1M count=1";
			QString insmodTest = "insmod g_mass_storage.ko  file=/tmp/a.img removable=1";
			totalCmd += QString("%1\n%2\n").arg(dd).arg(insmodTest);
		}
		
		//双模块
		QString file2 = "a" + currentSetInfo.imgName;
		QString doubleCmd = doubleImg ? QString(",/mnt/%2").arg(file2) : "";

		//挂载img命令
		QString mount = QString("mount -t cifs %1 /mnt -o username=%2,password=%3,rw")
			.arg(currentSetInfo.imgPath).arg(currentSetInfo.username).arg(currentSetInfo.password);				
 		QString insmod = QString("insmod g_mass_storage.ko file=/mnt/%1%2 removable=1").arg(currentSetInfo.imgName).arg(doubleCmd);

		//即时挂载镜像
// 		if (mountImgNow)
// 		{
// 			totalCmd += QString("%1\n%2\n").arg(mount).arg(insmod);
// 		}

		//写启动信息
		QString backup = QString("cp /etc/rc.local /etc/rc.local.bak");
		QString rmExit = QString("sed -i 's/exit 0/sleep 10/g' /etc/rc.local");
		QString addMount = QString("echo \"%1\" >> /etc/rc.local").arg(mount);
		QString addSleep2 = QString("echo \"sleep 5\" >> /etc/rc.local");
		QString addInsmod = QString("echo \"%1\" >> /etc/rc.local").arg(insmod);
		QString addExit = QString("echo \"exit 0\" >> /etc/rc.local");
		totalCmd += QString("%1\n%2\n%3\n%4\n%5\n%6\n").arg(backup).arg(rmExit)
			.arg(addMount).arg(addSleep2).arg(addInsmod).arg(addExit);

		//最后修改网络
		QString setIp = QString("uci set network.lan.ipaddr=%1").arg(currentSetInfo.ip);
		QString setMac = QString("uci set network.lan.macaddr=%1").arg(currentSetInfo.mac);
		QString setMask = QString("uci set network.lan.mask=%1").arg(currentSetInfo.mask);
		QString dhcp = "uci set dhcp.lan.ignore=1";
		QString commit = "uci commit";
		totalCmd += QString("%1\n%2\n%3\n%4\n%5").arg(setIp).arg(setMac).arg(setMask).arg(dhcp).arg(commit);

		//ui内容更新
		ui.textEdit_cmd->setText(totalCmd);
		ui.lineEdit_setip->setText(currentSetInfo.ip);
		ui.lineEdit_mac->setText(currentSetInfo.mac);
		ui.lineEdit_imgpath->setText(currentSetInfo.imgPath + "/" + currentSetInfo.imgName);
	}

	SshShell* InitNetworkDiskModel::newShellRunner(QSsh::SshConnectionParameters parm, QStringList cmdList)
	{
		SshShell* shell = new SshShell();
		shell->setShellList(cmdList);

		//
		connect(shell, SIGNAL(sshMessage(QString)), this, SLOT(handleSshMessage(QString)));
		connect(shell, SIGNAL(sshFinished()), this, SLOT(handleSshDisconnected()));
		connect(shell, SIGNAL(sshError(QSsh::SshError)), this, SLOT(handleSshErr(QSsh::SshError)));

		//连接执行命令
		shell->connectToHost(parm);
		return shell;
	}

	void InitNetworkDiskModel::enableButton(bool b)
	{
		ui.pushButton_exe->setEnabled(b);
		ui.pushButton_next->setEnabled(b);
		ui.pushButton_open->setEnabled(b);
	}

	void InitNetworkDiskModel::handleMountChange()
	{
		setCurrentCmdListToEdit(m_currentModelInfoIndex);
	}

	bool InitNetworkDiskModel::modelSetInfo::isAllEmpty()
	{
		if (ip.isEmpty() && mac.isEmpty() && imgPath.isEmpty() && username.isEmpty() && password.isEmpty())
		{
			return true;
		}
		return false;
	}
}
