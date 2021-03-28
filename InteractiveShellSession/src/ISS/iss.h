#pragma once
#include <string>
#include <sstream>
#include <stdio.h>
#include <Windows.h>
#include <functional>

namespace iss
{
	using ISSReadShellFn_t = std::function<void(std::string)>;
	using ISSWriteShellFn_t = std::function<std::string(void)>;

	class InteractiveShell
	{
	public:
		InteractiveShell();

		int		initialize();
		int		launch();
		void	shutdown();

		inline void set_shell_read_callback(ISSReadShellFn_t cb) { m_ReadShellCallback = cb; }
		inline void set_shell_write_callback(ISSWriteShellFn_t cb) { m_WriteShellCallback = cb; }

		static std::string interpret_return_code(int rc);

		~InteractiveShell();

	private:
		BYTE m_ShellBuffer[16384];

		HANDLE m_PipeInWrite	= NULL;
		HANDLE m_PipeInRead		= NULL;
		HANDLE m_PipeOutWrite	= NULL;
		HANDLE m_PipeOutRead	= NULL;

		PROCESS_INFORMATION pi = { 0 };

	private:
		ISSReadShellFn_t m_ReadShellCallback = nullptr;
		ISSWriteShellFn_t m_WriteShellCallback = nullptr;
	};
}
