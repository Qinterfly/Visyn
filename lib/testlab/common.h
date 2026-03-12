#pragma once

#include <string>
#include <vector>

namespace Testlab
{
	enum ResponseType
	{
		kUnknown,
		kDisp,
		kVeloc,
		kAccel,
		kForce
	};

	class IResponse
	{
	public:
		virtual ~IResponse() = default;

		// Data
		std::vector<double> keys;
		std::vector<double> realValues;
		std::vector<double> imagValues;

		// Info
		ResponseType type;
		std::wstring path;
		std::wstring originalRun;
		std::wstring name;
		std::wstring node;
		std::wstring component;
		std::wstring direction;
		std::wstring dimension;
		int channel;
		int numAverages;
		int sign;
		std::wstring transducer;
	};

	class IProject
	{
	public:
		virtual ~IProject() = default;

		virtual bool isValid() const = 0;
		virtual std::wstring getPath() = 0;

		// Section
		virtual std::wstring getActiveSection() = 0;
		virtual void createSection(std::wstring const& section, bool isSelect) = 0;
		virtual bool isSectionExist(std::wstring const& section) = 0;

		// Folder
		virtual void createFolder(std::wstring const& section, std::wstring const& folder) = 0;
		virtual bool isFolderExist(std::wstring const& section, std::wstring const& folder) = 0;

		// Responses
		virtual std::vector<IResponse*> getResponses(std::vector<std::wstring> const& paths) = 0;
		virtual std::vector<IResponse*> getSelectedResponses() = 0;
		virtual bool addResponses(std::vector<IResponse*> const& responses, std::wstring const& path) = 0;
	};
}

