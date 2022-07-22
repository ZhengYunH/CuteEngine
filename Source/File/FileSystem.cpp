#include "FileSystem.h"
#include "Core/IEntity.h"
#include "Core/IPrimitivesComponent.h"

// reference: https://www.boost.org/doc/libs/1_79_0/doc/html/property_tree.html

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


	bool SceneXmlParser::Load()
	{
		if (!XmlParser::Load())
			return false;
		auto world = GetPTree().get_child("World");
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

	void SceneXmlParser::Save()
	{
		pt::write_xml(mFileName_, GetPTree());
	}

	void Archive::BeginSection(const std::string& sectionName)
	{
		if (!mCurrentTreeNode_)
		{
			_BeforeSave();
			mDebugCurrentSection_ = sectionName;
		}
		else
		{
			mDebugCurrentSection_ = mDebugCurrentSection_ + "." + sectionName;
		}
		ArchiveTreeNode* newTreeNode = new ArchiveTreeNode(sectionName);
		if (mCurrentTreeNode_)
		{
			mCurrentTreeNode_->InsertChild(newTreeNode);
		}
		mCurrentTreeNode_ = newTreeNode;
	}

	void Archive::EndSection()
	{
		HYBRID_CHECK(mCurrentTreeNode_);
		mCurrentTreeNode_ = mCurrentTreeNode_->mParent_;
		if (!mCurrentTreeNode_->mParent_) // Root Node
		{
			mRootTreeNode_->Consolidate();
			Save();
			_AfterSave();
			return;
		}

		{
			size_t splitIdx = mDebugCurrentSection_.rfind('.');
			if (splitIdx != std::string::npos)
				mDebugCurrentSection_.erase(splitIdx);
			else
				mDebugCurrentSection_ = "";
		}
	}

	void Archive::_BeforeSave()
	{
		HYBRID_CHECK(mState_ == ARCHIVE_STATE::None);
		
		mState_ = Saving;
		mDebugCurrentSection_.clear();
		mCurrentTreeNode_ = mRootTreeNode_;
	}

	void Archive::_AfterSave()
	{
		mState_ = Saved;
		mRootTreeNode_->Clear();
		mCurrentTreeNode_ = nullptr;
	}

	namespace ArchiveTest
	{
		void test()
		{
			std::string filename = "Resource/files/test.xml";
			XmlParser ar(filename);
			ar.BeginSection("file");
			{
				ar.BeginSection("section1");
				{
					ar.AddItem("itemInt", 1);
					ar.AddItem("itemInt", 2);
					ar.AddItem("itemInt", 3);
					ar.AddItem("itemFloat", 1.0f);
					ar.AddItem("itemString", "abed");
				}
				ar.EndSection();

				ar.BeginSection("section1");
				{
					ar.AddItem("itemInt", 1);
					ar.AddItem("itemFloat2", 1.0f);
					ar.AddItem("itemString2", "abed");
				}
				ar.EndSection();
			}
			ar.EndSection();
		}
	}
}


