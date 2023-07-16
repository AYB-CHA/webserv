#pragma once
# include <string>
# include <map>
# include <vector>

class ABase {
	private:
		std::string root;
		std::string upload_path; //*
		std::map<std::string, bool> allowed_methods;
		std::vector<std::string> index;
		std::map<int, std::string> error_page;
		std::string client_max_body_size; //*

	public:
		virtual ~ABase() = 0;

		const std::string &getRoot(void) const;
		const std::string &getUploadPath(void) const;
		const std::map<std::string, bool> &getAllowedMethods(void) const;
		const std::vector<std::string> &getIndex(void) const;
		const std::map<int, std::string> &getErrorPage(void) const;
		const std::string &getClientMaxBodySize(void) const;

		void setRoot(std::string root);
		void setUploadPath(std::string upload_path);
		void setAllowedMethods(std::string method);
		void setIndex(std::string index);
		void setErrorPage(int status_code, std::string path);
		void setClientMaxBodySize(std::string max_body_size);
};
