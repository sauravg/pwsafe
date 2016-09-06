#include <gtest/gtest.h>

#include <sstream>

#include "./argutils.h"
#include "./search-internal.h"
#include "./safeutils-internal.h"

namespace {

using std::make_tuple;
std::wostringstream os;

// The fixture for testing Search dispatch function.
class SearchTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  SearchTest() {
    // You can do set-up work for each test here.
  }

  virtual ~SearchTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
    AddEntryWithFields(core, {make_tuple(CItemData::TITLE, L"SomeTitle"),
                              make_tuple(CItemData::EMAIL, L"test@example.com"),
                              make_tuple(CItemData::URL,   L"http://example.com"),
                              make_tuple(CItemData::GROUP, L"Example.Group"),
                              make_tuple(CItemData::USER,  L"ExampleUser"),
                              make_tuple(CItemData::NOTES, L"Line1\nLine2\nLine3"), }, os);
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
    core.ClearData();
  }

  // Objects declared here can be used by all tests in the test case for SearchTest.
  PWScore core;
  std::wstringstream os;
};

// Tests that the Split works correctly for single-char separators
TEST_F(SearchTest, SearchAndPrint) {
  UserArgs ua;

  ua.Operation = UserArgs::OpType::Search;
  ua.opArg = L"SomeTitle";

  SearchInternal(core, ua, os);

  EXPECT_NE( os.str().find(L"SomeTitle"), std::wstring::npos );
}

TEST_F(SearchTest, SearchAndDelete) {
  UserArgs ua;

  ua.Operation = UserArgs::OpType::Search;
  ua.opArg = L"SomeTitle";
  ua.confirmed = true;
  ua.SearchAction = UserArgs::Delete;

  SearchInternal(core, ua, os);

  EXPECT_EQ( core.GetNumEntries(), 0 );
}

TEST_F(SearchTest, SearchAndUpdate) {
  UserArgs ua;

  ua.Operation = UserArgs::OpType::Search;
  ua.opArg = L"SomeTitle";
  ua.confirmed = true;
  ua.SearchAction = UserArgs::Update;
  ua.fieldValues = { std::make_tuple(CItemData::EMAIL, L"e-mail=somerandomenewemail@nosuchdomain.com"),
                     std::make_tuple(CItemData::URL, L"nowaysuchadomainexists.pwsafe")
                  };

  SearchInternal(core, ua, os);

  EXPECT_EQ( core.GetNumEntries(), 1 );
  const CItemData &entry = core.GetEntryIter()->second;
  EXPECT_EQ( entry.GetEmail(), L"e-mail=somerandomenewemail@nosuchdomain.com" );
  EXPECT_EQ( entry.GetURL(), L"nowaysuchadomainexists.pwsafe" );
}

TEST_F(SearchTest, SearchAndClearFields) {
  UserArgs ua;

  ua.Operation = UserArgs::OpType::Search;
  ua.opArg = L"SomeTitle";
  ua.confirmed = true;
  ua.SearchAction = UserArgs::ClearFields;
  ua.opArg2 = L"Notes";

  SearchInternal(core, ua, os);

  EXPECT_EQ( core.GetNumEntries(), 1 );
  const CItemData &entry = core.GetEntryIter()->second;
  EXPECT_TRUE( entry.IsNotesEmpty() );
}

TEST_F(SearchTest, SearchAndChangePasswords) {
  UserArgs ua;

  ua.Operation = UserArgs::OpType::Search;
  ua.opArg = L"SomeTitle";
  ua.confirmed = true;
  ua.SearchAction = UserArgs::ChangePassword;

  const StringX before = core.GetEntryIter()->second.GetPassword();
  SearchInternal(core, ua, os);

  EXPECT_EQ( core.GetNumEntries(), 1 );
  const StringX after = core.GetEntryIter()->second.GetPassword();

  EXPECT_NE( before, after );
}

TEST_F(SearchTest, SearchConditionallyAndChangePasswords) {
  UserArgs ua;

  // Add an entry with a different group, but everything else same as the existing entry
  int added = AddEntryWithFields(core, {make_tuple(CItemData::TITLE, L"SomeTitle"),
                            make_tuple(CItemData::EMAIL, L"test@example.com"),
                            make_tuple(CItemData::URL,   L"http://example.com"),
                            make_tuple(CItemData::GROUP, L"Example.Group2"),
                            make_tuple(CItemData::USER,  L"ExampleUser"),
                            make_tuple(CItemData::NOTES, L"Line1\nLine2\nLine3"), }, os);

  EXPECT_EQ(added, PWScore::SUCCESS);

  ua.Operation = UserArgs::OpType::Search;
  ua.opArg = L"SomeTitle";
  ua.confirmed = true;
  ua.SetSubset(L"Group==Example.Group2");
  ua.SearchAction = UserArgs::ChangePassword;

  auto group2passwd = [this](const StringX& group) {
    auto itr = find_if(core.GetEntryIter(), core.GetEntryEndIter(),
            [&group](const ItemList_Pair& p) { return p.second.GetGroup() == group; });
    return itr == core.GetEntryEndIter()? StringX{}: itr->second.GetPassword();
  };

  const StringX pass1before = group2passwd(L"Example.Group");
  const StringX pass2before = group2passwd(L"Example.Group2");

  SearchInternal(core, ua, os);

  EXPECT_EQ( core.GetNumEntries(), 2 );
  const StringX pass1after = group2passwd(L"Example.Group");
  const StringX pass2after = group2passwd(L"Example.Group2");

  EXPECT_NE( pass2after, pass2before );
  EXPECT_EQ( pass1after, pass1before );
}

}  // namespace
