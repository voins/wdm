/**
 * Copyright (C) 2002, 2003 Alexey Voinov <voins@voins.program.ru>
 *
 * @file
 * Файл для запуска .hh-тестов
 */
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main(int argc, char **argv)
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest( registry.makeTest() );
	bool wasSucessful = runner.run("", false);
	return !wasSucessful;
}

