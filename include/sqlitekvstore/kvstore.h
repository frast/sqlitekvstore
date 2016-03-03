#pragma once

extern "C" {
#include <sqlite3.h>
}
#include <memory>

namespace sqlite_kv_store
{

namespace detail
{
inline void expects(bool cond) noexcept
{
	if (!cond)
		std::terminate();
}

inline void ensures(bool cond) noexcept
{
	if (!cond)
		std::terminate();
}

struct stmt_deleter
{
	void operator()(sqlite3_stmt *p) const noexcept
	{
		expects(SQLITE_OK == sqlite3_finalize(p));
	}
};

struct db_deleter
{
	void operator()(sqlite3 *p) const noexcept
	{
		expects(SQLITE_OK == sqlite3_close_v2(p));
	}
};

using stmt_ptr = ::std::unique_ptr<sqlite3_stmt, stmt_deleter>;
using db_ptr = ::std::unique_ptr<sqlite3, db_deleter>;

struct db_t
{
	db_t() noexcept
	{
		sqlite3 *db;
		expects(SQLITE_OK == sqlite3_open_v2("", &db,
		                                     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
		                                     nullptr));
		db_ = db_ptr{ db };
	}

	void exec(char const *sql) noexcept
	{
		expects(SQLITE_OK == sqlite3_exec(db_.get(), sql, nullptr, nullptr, nullptr));
	}

	sqlite3 *get() noexcept { return db_.get(); }

	db_ptr db_;
};

struct stmt_t
{
	template <::std::size_t N>
	stmt_t(db_t &db, char const (&sql)[N]) noexcept
	{
		sqlite3_stmt *stmt;
		expects(SQLITE_OK == sqlite3_prepare_v2(db.get(), sql, N, &stmt, nullptr));
		stmt_ = stmt_ptr{ stmt };
	}

	template <::std::size_t N>
	void bind(int i, char const (&v)[N]) noexcept
	{
		expects(SQLITE_OK == sqlite3_bind_text(stmt_.get(), i, v, N, SQLITE_STATIC));
	}

	void exec() noexcept { expects(SQLITE_DONE == sqlite3_step(stmt_.get())); }

	sqlite3_stmt *get() noexcept { return stmt_.get(); }

	stmt_ptr stmt_;
};
}

class kv_store
{
public:
	kv_store() noexcept : insert_(db_) {}

	template <::std::size_t N1, ::std::size_t N2>
	void insert(char const (&key)[N1], char const (&value)[N2]) noexcept
	{
		insert_.exec(key, value);
	}

private:
	struct create_db
	{
		create_db() noexcept
		{
			db_.exec(
			    u8R"*(
				CREATE TABLE kv(key, value);
			)*");
		}

		detail::db_t db_;
	};

	struct inserter
	{
		inserter(create_db &db) noexcept
		    : insert_(db.db_, "INSERT INTO kv(key, value) VALUES(?, ?)")

		{
		}

		template <::std::size_t N1, ::std::size_t N2>
		void exec(char const (&key)[N1], char const (&value)[N2]) noexcept
		{
			insert_.bind<N1>(1, key);
			insert_.bind<N2>(2, value);
			insert_.exec();
		}

		detail::stmt_t insert_;
	};

	create_db db_;
	inserter insert_;
};
}
