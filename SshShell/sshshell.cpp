#include "sshshell.h"
#include "ssh/sshremoteprocess.h"

namespace DiskModel {
	SshShell::SshShell(const SshConnectionParameters& param, QStringList& shellList, QObject *parent /*= 0*/)
		:QObject(parent)
	{
		setShellList(shellList);
		connectToHost(param);
	}

	SshShell::SshShell(QObject *parent /*= 0*/)
		:QObject(parent)
	{

	}

	SshShell::~SshShell()
	{
		m_sshConn->disconnectFromHost();
	}

	bool SshShell::connectToHost(const SshConnectionParameters& param)
	{
		if (param.host.isEmpty() || param.userName.isEmpty() || param.password.isEmpty())
		{
			return false;
		}
		
		if (m_sshConn.data() != NULL)
		{
			m_sshConn->disconnectFromHost();
		}

		m_sshConn.reset(new SshConnection(param));

		//connect
		connect(m_sshConn.data(), SIGNAL(connected()), this, SLOT(handleConnected()));
		connect(m_sshConn.data(), SIGNAL(dataAvailable(const QString&)), this, SLOT(handleSshMessage(const QString&)));
		connect(m_sshConn.data(), SIGNAL(error(QSsh::SshError)), this, SLOT(handleSshErr(QSsh::SshError)));
		connect(m_sshConn.data(), SIGNAL(disconnected()), this, SLOT(handleSshDisconnnected()));

		m_sshConn->connectToHost();
		return true;
	}

	void SshShell::createShell()
	{
		if (m_shellList.isEmpty())
		{
			return;
		}
		m_shell = m_sshConn->createRemoteShell();
		connect(m_shell.data(), SIGNAL(started()), this, SLOT(handleShellStarted()));
		m_shell->start();

	}

	void SshShell::handleConnected()
	{
		//连接成功
		qDebug() << "连接模块成功！" << endl;
		sshMessage(tr("*************连接成功*************"));
		createShell();		
	}

	void SshShell::handleSshMessage(const QString& msg)
	{
		qDebug() << msg << endl;
		sshMessage(qPrintable(msg));
	}

	void SshShell::handleSshErr(QSsh::SshError err)
	{
		qDebug() << err << endl;
		sshError(err);
	}

	void SshShell::handleSshDisconnnected()
	{
		qDebug() << "连接断开！" << endl;
	}

	void SshShell::handleShellStarted()
	{
		foreach(QString cmd, m_shellList)
		{
			if (cmd.trimmed().isEmpty())
			{
				continue;
			}
			sshMessage(QString("===>current shell: %1").arg(cmd));
			if (!cmd.endsWith("\n"))
			{
				cmd += "\n";
			}
			m_shell->write(cmd.toStdString().c_str());
		}
		sshFinished();
	}
}
