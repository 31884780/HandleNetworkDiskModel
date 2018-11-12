#pragma once
#pragma execution_character_set("utf-8")

#include "sshshell_global.h"
#include "ssh\sshconnection.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSharedPointer>

using namespace QSsh;
namespace DiskModel {
	class SSHSHELL_EXPORT SshShell : public QObject
	{
		Q_OBJECT

	public:
		SshShell(QObject *parent = 0);
		SshShell(const SshConnectionParameters& param, QStringList& shellList, QObject *parent = 0);
		~SshShell();

		bool connectToHost(const SshConnectionParameters& param);
		void setShellList(const QStringList& shellList) { m_shellList = shellList; }
	signals:
		void sshMessage(QString sshMsg);
		void sshError(QSsh::SshError err);
		void sshFinished();
	private:
		
		void createShell();
	private slots :
		void handleConnected();
		void handleSshMessage(const QString& msg);
		void handleSshErr(QSsh::SshError err);
		void handleSshDisconnnected();
		void handleShellStarted();
	private:
		QSharedPointer<SshConnection> m_sshConn;
		QSharedPointer<SshRemoteProcess> m_shell;
		//SshConnectionParameters m_sshParam;
		QStringList m_shellList;
	};
}
