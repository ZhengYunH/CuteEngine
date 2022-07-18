#include "FileSystem.h"
#include "Core/IEntity.h"
#include "Core/IPrimitivesComponent.h"

namespace zyh
{
	void debug_settings::load(const std::string& filename)
	{
		pt::ptree tree;
		pt::read_xml(filename, tree);
		m_file = tree.get<std::string>("debug.filename");
		m_level = tree.get("debug.level", 0);

		BOOST_FOREACH(pt::ptree::value_type & v, tree.get_child("debug.modules")) {
			m_modules.insert(v.second.data());
		}
	}

	void debug_settings::save(const std::string& filename)
	{
		pt::ptree tree;

		tree.put("debug.filename", m_file);
		tree.put("debug.level", m_level);

		BOOST_FOREACH(const std::string & name, m_modules)
			tree.add("debug.modules.module", name);

		pt::write_xml(filename, tree);
	}


	bool SceneXmlParser::load()
	{
		if (!XmlParser::load())
			return false;
		auto world = tree.get_child("World");
		BOOST_FOREACH(pt::ptree::value_type & v, world.get_child("Entities"))
		{
			auto entityElement = v.second;
			IEntity* entity = new IEntity();
			auto primitiveComponentElement = entityElement.get_child("IPrimitivesComponent");
			EPrimitiveType primitiveType = EPrimitiveType(primitiveComponentElement.get<int>("EPrimitiveType", int(EPrimitiveType::NONE)));
			std::string primitiveResource = primitiveComponentElement.get<std::string>("ResourcePath", "");

			HYBRID_CHECK(primitiveType != EPrimitiveType::NONE);

			IComponent* comp = entity->AddComponent<IPrimitivesComponent>(primitiveType, primitiveResource);
			entity->AddUpdateTransformList(comp);
			mEntities_.push_back(entity);
		}
		
		return true;
	}

	void SceneXmlParser::save()
	{

	}

}


