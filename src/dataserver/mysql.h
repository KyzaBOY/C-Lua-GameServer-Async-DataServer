#pragma once
#include <string>
#include <vector>

namespace SQLSystem {

	bool Connect(const std::string& odbc, const std::string& user, const std::string& pass);
	void Disconnect();

	bool Query(const std::string& sql);
	bool Fetch();
	void Close();

	int GetNumber(int column);
	std::string GetString(int column);
	float GetFloat(int column);
	bool GetResult();

	bool IsConnected();
}
