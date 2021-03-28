#include "ISS/iss.h"
#include <memory>
#include <iostream>

void print_shell_output(std::string str)
{
	printf("%s", str.c_str());
}

std::string get_user_input()
{
	std::string input;
	std::getline(std::cin, input);

	return input;
}

int main(int argc, char* argv[])
{
	std::unique_ptr<iss::InteractiveShell> shell = std::make_unique<iss::InteractiveShell>();
	shell->set_shell_read_callback(print_shell_output);
	shell->set_shell_write_callback(get_user_input);

	int rc = shell->initialize();
	printf("Initialize Status: %s\n", iss::InteractiveShell::interpret_return_code(rc).c_str());

	rc = shell->launch();
	printf("Shell Running Status: %s\n", iss::InteractiveShell::interpret_return_code(rc).c_str());
	return 0;
}
