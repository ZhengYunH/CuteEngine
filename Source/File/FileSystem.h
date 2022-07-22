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

	enum ARCHIVE_STATE
	{
		None = 0,
		Loading = 1,
		Loaded = 2,
		Saving = 3,
		Saved = 4,
	};

	struct ArchiveTreeNode
	{
		ArchiveTreeNode()
		{

		}

		ArchiveTreeNode(std::string key)
			: mKey_(key)
		{

		}

		void InsertChild(ArchiveTreeNode* node)
		{
			mModified = true;
			node->mParent_ = this;
			if (!mChild_)
			{
				this->mChild_ = node;
				return;
			}

			ArchiveTreeNode* child = mChild_;
			while (child->mNextSibling_)
			{
				child = child->mNextSibling_;
			}
			child->mNextSibling_ = node;
			node->mPreSibling_ = child;
		}
		
		void Consolidate()
		{
			if (!mModified)
				return;
			auto child = mChild_;
			while (child)
			{
				child->Consolidate();
				mTree_.add_child(child->mKey_, child->mTree_);
				child = child->mNextSibling_;
			}
			mModified = false;
		}

		void Clear()
		{
			auto child = mChild_;
			while (child)
			{
				child->Clear();
				auto current = child;
				child = child->mNextSibling_;
				delete current;
			}
		}

		template <typename Type> void AddItem(std::string key, Type value)
		{
			mTree_.add<Type>(key, value);
			mModified = true;
		}

		std::string mKey_;
		pt::ptree mTree_;
		ArchiveTreeNode* mParent_{ nullptr };
		ArchiveTreeNode* mChild_{ nullptr };
		ArchiveTreeNode* mNextSibling_ {nullptr};
		ArchiveTreeNode* mPreSibling_{ nullptr };
		bool	mModified{ false };
	};

	class Archive
	{
		friend struct ArchiveTreeNode;
		using TArchivePair = std::pair<std::string, std::string>;

	public:
		Archive() 
		{
			mRootTreeNode_ = new ArchiveTreeNode();
		}

		virtual bool IsLoading() { return mState_ == Loading; }
		virtual bool IsSaving() { return mState_ == Saving; };

		virtual bool Load() = 0;
		virtual void Save() = 0;

		void BeginSection(const std::string& sectionName);
		
		template <typename Type> void AddItem(std::string key, Type value)
		{
			mCurrentTreeNode_->AddItem<Type>(key, value);
		}

		void EndSection();

		const std::string& GetCurrentSection() const
		{
			return mDebugCurrentSection_;
		}

		pt::ptree& GetPTree() { return mRootTreeNode_->mTree_; }
	protected:
		void _BeforeSave();
		void _AfterSave();

	protected:
		ARCHIVE_STATE mState_;
		std::string mFileName_;
		std::string mDebugCurrentSection_;
		ArchiveTreeNode* mCurrentTreeNode_;
		ArchiveTreeNode* mRootTreeNode_;
	};

	class XmlParser : public virtual Archive
	{
	public:
		XmlParser(const std::string& filename)
		{
			mFileName_ = filename;
		}

	// implement Archive
	public:
		virtual bool Load() override
		{
			if (!GetPTree().empty()) // already load file
			{
				return false;
			}
			pt::read_xml(mFileName_, GetPTree());
			return true;
		}

		virtual void Save() override
		{
			pt::write_xml(mFileName_, GetPTree());
		}

		template <typename _Type>
		_Type get(const std::string& name, _Type defaultValue = _Type(0))
		{
			HYBRID_CHECK(!GetPTree().empty());
			return GetPTree().get<_Type>(name, defaultValue);
		}
	};

	class IEntity;
	class SceneXmlParser : public XmlParser
	{
	public:
		SceneXmlParser(const std::string& filename) : XmlParser(filename)
		{
		}

		virtual bool Load() override;
		virtual void Save() override;

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

	namespace ArchiveTest
	{
		void test();
	}
}

