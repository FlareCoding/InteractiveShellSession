#include "iss.h"

namespace iss
{
	InteractiveShell::InteractiveShell()
	{
	}

	InteractiveShell::~InteractiveShell()
	{
		shutdown();
	}

	int InteractiveShell::initialize()
	{
		SECURITY_ATTRIBUTES sats = { 0 };
		sats.nLength = sizeof(sats);
		sats.bInheritHandle = TRUE;
		sats.lpSecurityDescriptor = NULL;

		if (!CreatePipe(&m_PipeOutRead, &m_PipeOutWrite, &sats, 0))
			return 13;

		if (!CreatePipe(&m_PipeInRead, &m_PipeInWrite, &sats, 0))
			return 15;

		return 0;
	}

	int InteractiveShell::launch()
	{
		STARTUPINFO sti = { 0 };
		sti.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sti.wShowWindow = SW_HIDE;
		sti.hStdInput = m_PipeInRead;
		sti.hStdOutput = m_PipeOutWrite;
		sti.hStdError = m_PipeOutWrite;

#pragma warning (push)
#pragma warning (disable: 6277)
		if (!CreateProcess(NULL, (char*)"cmd.exe", NULL, NULL, TRUE, 0, NULL, NULL, &sti, &pi))
			return 5;
#pragma warning (pop)

		int return_code = 0;
		DWORD bytes_read, bytes_written, bytes_available;

		while (true)
		{
			DWORD excode;
			GetExitCodeProcess(pi.hProcess, &excode);
			if (excode != STILL_ACTIVE)
				return 7;

			Sleep(500);

			if (!PeekNamedPipe(m_PipeOutRead, m_ShellBuffer, sizeof(m_ShellBuffer), &bytes_read, &bytes_available, NULL))
				return_code = 113;

			if (bytes_read)
			{
				std::ostringstream result_stream;

				do
				{
					ZeroMemory(m_ShellBuffer, sizeof(m_ShellBuffer));

					if (!ReadFile(m_PipeOutRead, m_ShellBuffer, sizeof(m_ShellBuffer), &bytes_read, NULL) || !bytes_read)
						return_code = 115;

					result_stream << m_ShellBuffer;

					if (return_code)
						break;
				}
				while (bytes_read >= sizeof(m_ShellBuffer));

				std::string result = result_stream.str();
				if (m_ReadShellCallback)
					m_ReadShellCallback(result);

				if (!return_code)
				{
					ZeroMemory(m_ShellBuffer, sizeof(m_ShellBuffer));

					if (m_WriteShellCallback)
					{
						std::string input = m_WriteShellCallback();
						if (input.back() != '\n')
							input += '\n';

						memcpy(m_ShellBuffer, input.data(), input.length());

						if (!_strnicmp((char*)m_ShellBuffer, "exit", 4))
							return_code = 2;

						if (!WriteFile(m_PipeInWrite, m_ShellBuffer, strlen((char*)m_ShellBuffer), &bytes_written, NULL))
							return_code = 117;
					}
				}

				if (return_code)
					break;
			}
		}

		return return_code;
	}
	
	void InteractiveShell::shutdown()
	{
		if (m_PipeInWrite != NULL) CloseHandle(m_PipeInWrite);
		if (m_PipeInRead != NULL) CloseHandle(m_PipeInRead);
		if (m_PipeOutWrite != NULL) CloseHandle(m_PipeOutWrite);
		if (m_PipeOutRead != NULL) CloseHandle(m_PipeOutRead);
		if (pi.hProcess != NULL) CloseHandle(pi.hProcess);
		if (pi.hThread != NULL) CloseHandle(pi.hThread);
	}

	std::string InteractiveShell::interpret_return_code(int rc)
	{
		switch (rc)
		{
		case 0:
			return "Success";
		case 2:
			return "\"exit\" command was used to exit the shell process";
		case 5:
			return "Failed to create cmd.exe process";
		case 7:
			return "cmd.exe child process is no longer active";
		case 13:
			return "Failed to create output pipes";
		case 15:
			return "Failed to create input pipes";
		case 113:
			return "cmd.exe child process had no output available";
		case 115:
			return "Error reading output from child cmd.exe process";
		case 117:
			return "Error writing input into child cmd.exe process";
		default:
			return "Unknown ISS Error: " + std::to_string(rc);
		}
	}
}
