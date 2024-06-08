#include <gtest/gtest.h>
#include "core/memory.h"
#include "core/stl/shared_ptr.h"
#include "core/stl/unique_ptr.h"

using namespace Sapfire;

TEST(Memory_Tests, mem_new) {
	auto& categories = mem::categories();
	EXPECT_EQ(0, categories[mem::ENUM::Engine_Core]);
	auto* p_int = mem_new(mem::ENUM::Engine_Core) int(0);
	EXPECT_EQ(0, *p_int);
	EXPECT_EQ(4, categories[mem::ENUM::Engine_Core]);
	mem_delete(p_int);
	EXPECT_EQ(nullptr, p_int);
	EXPECT_EQ(0, categories[mem::ENUM::Engine_Core]);
}

TEST(Memory_Tests, unique_ptr) {
	auto& categories = mem::categories();
	EXPECT_EQ(0, categories[mem::ENUM::Engine_Core]);
	{
		stl::unique_ptr<int> p_int = stl::make_unique<int>(mem::ENUM::Engine_Core, 0);
		EXPECT_EQ(0, *p_int);
		EXPECT_EQ(4, categories[mem::ENUM::Engine_Core]);
	}
	EXPECT_EQ(0, categories[mem::ENUM::Engine_Core]);
}

TEST(Memory_Tests, shared_ptr) {
	auto& categories = mem::categories();
	EXPECT_EQ(0, categories[mem::ENUM::Engine_Core]);
	{
		stl::shared_ptr<int> p_int = stl::make_shared<int>(mem::ENUM::Engine_Core, 0);
		EXPECT_EQ(0, *p_int);
		EXPECT_EQ(4, categories[mem::ENUM::Engine_Core]);
		EXPECT_EQ(1, p_int.use_count());
		{
			stl::shared_ptr<int> copy = p_int;
			EXPECT_EQ(2, p_int.use_count());
			EXPECT_EQ(2, copy.use_count());
			EXPECT_EQ(4, categories[mem::ENUM::Engine_Core]);
		}
		EXPECT_EQ(1, p_int.use_count());
		EXPECT_EQ(4, categories[mem::ENUM::Engine_Core]);
	}
	EXPECT_EQ(0, categories[mem::ENUM::Engine_Core]);
}
