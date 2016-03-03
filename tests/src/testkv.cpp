#include "stdafx.h"
#include "sqlitekvstore\kvstore.h"

using namespace sqlite_kv_store;

TEST(kv, create)
{
	kv_store kv;
	kv.insert("Hallo", "Welt");
}