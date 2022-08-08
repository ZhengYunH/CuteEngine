#include "FileSystem.h"
#include "Core/IEntity.h"
#include "Core/IPrimitivesComponent.h"
#include "Core/TerrainComponent.h"
#include "Math/Matrix4x3.h"

#include <iostream>

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
			auto componentsElement = entityElement.get_child("Components");
			auto primitiveComponentOptional = componentsElement.get_child_optional("IPrimitivesComponent");
			if (primitiveComponentOptional.has_value())
			{
				auto primitiveComponentElement = primitiveComponentOptional.get();

				EPrimitiveType primitiveType = EPrimitiveType(primitiveComponentElement.get<int>("EPrimitiveType", int(EPrimitiveType::NONE)));
				std::string primitiveResource = primitiveComponentElement.get<std::string>("ResourcePath", "");

				HYBRID_CHECK(primitiveType != EPrimitiveType::NONE);

				IComponent* comp = entity->AddComponent<IPrimitivesComponent>(primitiveType, primitiveResource);
				entity->AddUpdateTransformList(comp);
			}
			auto terrainComponentOptional = componentsElement.get_child_optional("TerrainComponent");
			if (terrainComponentOptional.has_value())
			{
				auto terrainComponentElement = terrainComponentOptional.get();
				IComponent* comp = entity->AddComponent<TerrainComponent>();
				entity->AddUpdateTransformList(comp);
			}
			
			Matrix4x3 mat = entityElement.get<Matrix4x3>("Transform");
			entity->SetTransform(mat);
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
		_BeginSection(sectionName);
	}

	void Archive::_BeginSection(const std::string& sectionName)
	{
		if (mUsage_ == ARCHIVE_USAGE::SAVE)
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
		else if (mUsage_ == ARCHIVE_USAGE::LOAD)
		{
			Unimplement();
		}
		else
		{
			Unimplement();
		}

	}

	void Archive::EndSection()
	{
		if (mUsage_ == SAVE)
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
		else if (mUsage_ == LOAD)
		{
			Unimplement();
		}
		else
		{
			Unimplement();
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

	void Archive::_BeforeLoad()
	{
		mState_ = Loading;
	}

	void Archive::_AfterLoad()
	{
		mState_ = Loaded;
	}

	void Archive::_TryEndSection(const std::string& sectionName)
	{
		// this may cause error check when two subtree has same name, 
		// but i ignore this situation
		if (mCurrentTreeNode_ && mCurrentTreeNode_->mKey_ == sectionName)
		{
			EndSection();
		}
	}

	class ArchiveTest
	{
	public:
		Matrix4x3 testMat;
		int i;
		float f;
		std::string str;
		std::string filename;

		ArchiveTest()
		{
			testMat.SetTranslation(Vector3(3, 4, 5));
			i = 1;
			f = 2.f;
			str = "abed";
			std::string filename = "Resource/files/test.xml";
		}

		void commonOp(XmlParser& ar)
		{
			ar.BeginSection("file");
			{
				ar.BeginSection("section1");
				{
					ar.AddItem("itemInt", i);
					ar.AddItem("itemInt", i);
					ar.AddItem("itemInt", i);
					ar.AddItem("itemFloat", f);
					ar.AddItem("itemString", str);
					ar.AddItem("mat", testMat);
				}
				ar.EndSection();

				ar.BeginSection("section1");
				{
					ar.AddItem("itemInt", i);
					ar.AddItem("itemFloat2", f);
				}
				ar.EndSection();
			}
			ar.EndSection();
		}

		void testSave()
		{
			XmlParser ar(filename);
			commonOp(ar);
		}

		void testLoad()
		{
		}
	};
}

