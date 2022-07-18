#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/foreach.hpp"
#include <string>
#include <set>
#include <exception>
#include <iostream>
#include <map>

#include "Common/Config.h"
namespace pt = boost::property_tree;

namespace zyh
{

	struct debug_settings
	{
		std::string m_file;               // log filename
		int m_level;                      // debug level
		std::set<std::string> m_modules;  // modules where logging is enabled
		void load(const std::string& filename);
		void save(const std::string& filename);
	};

	class XmlParser
	{
	public:
		XmlParser(const std::string& filename)
		{
			mFileName = filename;
		}

	public:
		virtual bool load()
		{
			if (!tree.empty()) // already load file
			{
				return false;
			}
			pt::read_xml(mFileName, tree);
			return true;
		}

		virtual void save()
		{

		}

		template <typename _Type>
		_Type get(const std::string& name, _Type defaultValue = _Type(0))
		{
			HYBRID_CHECK(!tree.empty());
			return tree.get<_Type>(name, defaultValue);
		}

	protected:
		std::string mFileName;
		pt::ptree tree;
	};

	class IEntity;
	class SceneXmlParser : public XmlParser
	{
	public:
		static void TestDebugFile()
		{
			static debug_settings debug;
			debug.load("Resource/files/debug.xml");
			debug.m_level = 111;
			debug.save("Resource/files/debug.xml");
		}

	public:
		SceneXmlParser(const std::string& filename) : XmlParser(filename)
		{
		}

		virtual bool load() override;
		virtual void save() override;

	public:
		std::vector<IEntity*>& GetEntities()
		{
			return mEntities_;
		}

	protected:
		std::vector<IEntity*> mEntities_;
	};


	class LogManager
	{

	};
}

