// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: jschorr@google.com (Joseph Schorr)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <math.h>
#include <stdlib.h>
#include <limits>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/unittest.pb.h>
#include <google/protobuf/unittest_mset.pb.h>
#include <google/protobuf/test_util.h>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/testing/file.h>
#include <google/protobuf/testing/googletest.h>
#include <gtest/gtest.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>

namespace google {
namespace protobuf {

// Can't use an anonymous namespace here due to brokenness of Tru64 compiler.
namespace text_format_unittest {

inline bool IsNaN(double value) {
  // NaN is never equal to anything, even itself.
  return value != value;
}

// A basic string with different escapable characters for testing.
const string kEscapeTestString =
  "\"A string with ' characters \n and \r newlines and \t tabs and \001 "
  "slashes \\ and  multiple   spaces";

// A representation of the above string with all the characters escaped.
const string kEscapeTestStringEscaped =
  "\"\\\"A string with \\' characters \\n and \\r newlines "
  "and \\t tabs and \\001 slashes \\\\ and  multiple   spaces\"";

class TextFormatTest : public testing::Test {
 public:
  static void SetUpTestCase() {
    File::ReadFileToStringOrDie(
        TestSourceDir()
        + "/google/protobuf/testdata/text_format_unittest_data.txt",
        &static_proto_debug_string_);
  }

  TextFormatTest() : proto_debug_string_(static_proto_debug_string_) {}

 protected:
  // Debug string read from text_format_unittest_data.txt.
  const string proto_debug_string_;
  unittest::TestAllTypes proto_;

 private:
  static string static_proto_debug_string_;
};
string TextFormatTest::static_proto_debug_string_;

class TextFormatExtensionsTest : public testing::Test {
 public:
  static void SetUpTestCase() {
    File::ReadFileToStringOrDie(
        TestSourceDir()
        + "/google/protobuf/testdata/"
          "text_format_unittest_extensions_data.txt",
        &static_proto_debug_string_);
  }

  TextFormatExtensionsTest()
      : proto_debug_string_(static_proto_debug_string_) {}

 protected:
  // Debug string read from text_format_unittest_data.txt.
  const string proto_debug_string_;
  unittest::TestAllExtensions proto_;

 private:
  static string static_proto_debug_string_;
};
string TextFormatExtensionsTest::static_proto_debug_string_;


TEST_F(TextFormatTest, Basic) {
  TestUtil::SetAllFields(&proto_);
  EXPECT_EQ(proto_debug_string_, proto_.DebugString());
}

TEST_F(TextFormatExtensionsTest, Extensions) {
  TestUtil::SetAllExtensions(&proto_);
  EXPECT_EQ(proto_debug_string_, proto_.DebugString());
}

TEST_F(TextFormatTest, ShortDebugString) {
  proto_.set_optional_int32(1);
  proto_.set_optional_string("hello");
  proto_.mutable_optional_nested_message()->set_bb(2);
  proto_.mutable_optional_foreign_message();

  EXPECT_EQ("optional_int32: 1 optional_string: \"hello\" "
            "optional_nested_message { bb: 2 } "
            "optional_foreign_message { }",
            proto_.ShortDebugString());
}

TEST_F(TextFormatTest, StringEscape) {
  // Set the string value to test.
  proto_.set_optional_string(kEscapeTestString);

  // Get the DebugString from the proto.
  string debug_string = proto_.DebugString();

  // Hardcode a correct value to test against.
  string correct_string = "optional_string: "
      + kEscapeTestStringEscaped
       + "\n";

  // Compare.
  EXPECT_EQ(correct_string, debug_string);

  string expected_short_debug_string = "optional_string: "
      + kEscapeTestStringEscaped;
  EXPECT_EQ(expected_short_debug_string, proto_.ShortDebugString());
}

TEST_F(TextFormatTest, PrintUnknownFields) {
  // Test printing of unknown fields in a message.

  unittest::TestEmptyMessage message;
  UnknownFieldSet* unknown_fields = message.mutable_unknown_fields();
  UnknownField* field5 = unknown_fields->AddField(5);

  field5->add_varint(1);
  field5->add_fixed32(2);
  field5->add_fixed64(3);
  field5->add_length_delimited("4");
  field5->add_group()->AddField(10)->add_varint(5);

  UnknownField* field8 = unknown_fields->AddField(8);
  field8->add_varint(1);
  field8->add_varint(2);
  field8->add_varint(3);

  EXPECT_EQ(
    "5: 1\n"
    "5: 0x00000002\n"
    "5: 0x0000000000000003\n"
    "5: \"4\"\n"
    "5 {\n"
    "  10: 5\n"
    "}\n"
    "8: 1\n"
    "8: 2\n"
    "8: 3\n",
    message.DebugString());
}

TEST_F(TextFormatTest, PrintUnknownMessage) {
  // Test heuristic printing of messages in an UnknownFieldSet.

  protobuf_unittest::TestAllTypes message;

  // Cases which should not be interpreted as sub-messages.

  // 'a' is a valid FIXED64 tag, so for the string to be parseable as a message
  // it should be followed by 8 bytes.  Since this string only has two
  // subsequent bytes, it should be treated as a string.
  message.add_repeated_string("abc");

  // 'd' happens to be a valid ENDGROUP tag.  So,
  // UnknownFieldSet::MergeFromCodedStream() will successfully parse "def", but
  // the ConsumedEntireMessage() check should fail.
  message.add_repeated_string("def");

  // A zero-length string should never be interpreted as a message even though
  // it is technically valid as one.
  message.add_repeated_string("");

  // Case which should be interpreted as a sub-message.

  // An actual nested message with content should always be interpreted as a
  // nested message.
  message.add_repeated_nested_message()->set_bb(123);

  string data;
  message.SerializeToString(&data);

  string text;
  UnknownFieldSet unknown_fields;
  EXPECT_TRUE(unknown_fields.ParseFromString(data));
  EXPECT_TRUE(TextFormat::PrintUnknownFieldsToString(unknown_fields, &text));
  EXPECT_EQ(
    "44: \"abc\"\n"
    "44: \"def\"\n"
    "44: \"\"\n"
    "48 {\n"
    "  1: 123\n"
    "}\n",
    text);
}

TEST_F(TextFormatTest, ParseBasic) {
  io::ArrayInputStream input_stream(proto_debug_string_.data(),
                                    proto_debug_string_.size());
  TextFormat::Parse(&input_stream, &proto_);
  TestUtil::ExpectAllFieldsSet(proto_);
}

TEST_F(TextFormatExtensionsTest, ParseExtensions) {
  io::ArrayInputStream input_stream(proto_debug_string_.data(),
                                    proto_debug_string_.size());
  TextFormat::Parse(&input_stream, &proto_);
  TestUtil::ExpectAllExtensionsSet(proto_);
}

TEST_F(TextFormatTest, ParseStringEscape) {
  // Create a parse string with escpaed characters in it.
  string parse_string = "optional_string: "
      + kEscapeTestStringEscaped
      + "\n";

  io::ArrayInputStream input_stream(parse_string.data(),
                                    parse_string.size());
  TextFormat::Parse(&input_stream, &proto_);

  // Compare.
  EXPECT_EQ(kEscapeTestString, proto_.optional_string());
}

TEST_F(TextFormatTest, ParseFloatWithSuffix) {
  // Test that we can parse a floating-point value with 'f' appended to the
  // end.  This is needed for backwards-compatibility with proto1.

  // Have it parse a float with the 'f' suffix.
  string parse_string = "optional_float: 1.0f\n";

  io::ArrayInputStream input_stream(parse_string.data(),
                                    parse_string.size());

  TextFormat::Parse(&input_stream, &proto_);

  // Compare.
  EXPECT_EQ(1.0, proto_.optional_float());
}

TEST_F(TextFormatTest, Comments) {
  // Test that comments are ignored.

  string parse_string = "optional_int32: 1  # a comment\n"
                        "optional_int64: 2  # another comment";

  io::ArrayInputStream input_stream(parse_string.data(),
                                    parse_string.size());

  TextFormat::Parse(&input_stream, &proto_);

  // Compare.
  EXPECT_EQ(1, proto_.optional_int32());
  EXPECT_EQ(2, proto_.optional_int64());
}

TEST_F(TextFormatTest, OptionalColon) {
  // Test that we can place a ':' after the field name of a nested message,
  // even though we don't have to.

  string parse_string = "optional_nested_message: { bb: 1}\n";

  io::ArrayInputStream input_stream(parse_string.data(),
                                    parse_string.size());

  TextFormat::Parse(&input_stream, &proto_);

  // Compare.
  EXPECT_TRUE(proto_.has_optional_nested_message());
  EXPECT_EQ(1, proto_.optional_nested_message().bb());
}

// Some platforms (e.g. Windows) insist on padding the exponent to three
// digits when one or two would be just fine.
static string RemoveRedundantZeros(string text) {
  text = StringReplace(text, "e+0", "e+", true);
  text = StringReplace(text, "e-0", "e-", true);
  return text;
}

TEST_F(TextFormatTest, PrintExotic) {
  unittest::TestAllTypes message;

  // Note:  In C, a negative integer literal is actually the unary negation
  //   operator being applied to a positive integer literal, and
  //   9223372036854775808 is outside the range of int64.  However, it is not
  //   outside the range of uint64.  Confusingly, this means that everything
  //   works if we make the literal unsigned, even though we are negating it.
  message.add_repeated_int64(-GOOGLE_ULONGLONG(9223372036854775808));
  message.add_repeated_uint64(GOOGLE_ULONGLONG(18446744073709551615));
  message.add_repeated_double(123.456);
  message.add_repeated_double(1.23e21);
  message.add_repeated_double(1.23e-18);
  message.add_repeated_double(std::numeric_limits<double>::infinity());
  message.add_repeated_double(-std::numeric_limits<double>::infinity());
  message.add_repeated_double(std::numeric_limits<double>::quiet_NaN());
  message.add_repeated_string(string("\000\001\a\b\f\n\r\t\v\\\'\"", 12));

  // Fun story:  We used to use 1.23e22 instead of 1.23e21 above, but this
  //   seemed to trigger an odd case on MinGW/GCC 3.4.5 where GCC's parsing of
  //   the value differed from strtod()'s parsing.  That is to say, the
  //   following assertion fails on MinGW:
  //     assert(1.23e22 == strtod("1.23e22", NULL));
  //   As a result, SimpleDtoa() would print the value as
  //   "1.2300000000000001e+22" to make sure strtod() produce the exact same
  //   result.  Our goal is to test runtime parsing, not compile-time parsing,
  //   so this wasn't our problem.  It was found that using 1.23e21 did not
  //   have this problem, so we switched to that instead.

  EXPECT_EQ(
    "repeated_int64: -9223372036854775808\n"
    "repeated_uint64: 18446744073709551615\n"
    "repeated_double: 123.456\n"
    "repeated_double: 1.23e+21\n"
    "repeated_double: 1.23e-18\n"
    "repeated_double: inf\n"
    "repeated_double: -inf\n"
    "repeated_double: nan\n"
    "repeated_string: \"\\000\\001\\007\\010\\014\\n\\r\\t\\013\\\\\\'\\\"\"\n",
    RemoveRedundantZeros(message.DebugString()));
}

TEST_F(TextFormatTest, PrintFloatPrecision) {
  unittest::TestAllTypes message;

  message.add_repeated_float(1.2);
  message.add_repeated_float(1.23);
  message.add_repeated_float(1.234);
  message.add_repeated_float(1.2345);
  message.add_repeated_float(1.23456);
  message.add_repeated_float(1.2e10);
  message.add_repeated_float(1.23e10);
  message.add_repeated_float(1.234e10);
  message.add_repeated_float(1.2345e10);
  message.add_repeated_float(1.23456e10);
  message.add_repeated_double(1.2);
  message.add_repeated_double(1.23);
  message.add_repeated_double(1.234);
  message.add_repeated_double(1.2345);
  message.add_repeated_double(1.23456);
  message.add_repeated_double(1.234567);
  message.add_repeated_double(1.2345678);
  message.add_repeated_double(1.23456789);
  message.add_repeated_double(1.234567898);
  message.add_repeated_double(1.2345678987);
  message.add_repeated_double(1.23456789876);
  message.add_repeated_double(1.234567898765);
  message.add_repeated_double(1.2345678987654);
  message.add_repeated_double(1.23456789876543);
  message.add_repeated_double(1.2e100);
  message.add_repeated_double(1.23e100);
  message.add_repeated_double(1.234e100);
  message.add_repeated_double(1.2345e100);
  message.add_repeated_double(1.23456e100);
  message.add_repeated_double(1.234567e100);
  message.add_repeated_double(1.2345678e100);
  message.add_repeated_double(1.23456789e100);
  message.add_repeated_double(1.234567898e100);
  message.add_repeated_double(1.2345678987e100);
  message.add_repeated_double(1.23456789876e100);
  message.add_repeated_double(1.234567898765e100);
  message.add_repeated_double(1.2345678987654e100);
  message.add_repeated_double(1.23456789876543e100);

  EXPECT_EQ(
    "repeated_float: 1.2\n"
    "repeated_float: 1.23\n"
    "repeated_float: 1.234\n"
    "repeated_float: 1.2345\n"
    "repeated_float: 1.23456\n"
    "repeated_float: 1.2e+10\n"
    "repeated_float: 1.23e+10\n"
    "repeated_float: 1.234e+10\n"
    "repeated_float: 1.2345e+10\n"
    "repeated_float: 1.23456e+10\n"
    "repeated_double: 1.2\n"
    "repeated_double: 1.23\n"
    "repeated_double: 1.234\n"
    "repeated_double: 1.2345\n"
    "repeated_double: 1.23456\n"
    "repeated_double: 1.234567\n"
    "repeated_double: 1.2345678\n"
    "repeated_double: 1.23456789\n"
    "repeated_double: 1.234567898\n"
    "repeated_double: 1.2345678987\n"
    "repeated_double: 1.23456789876\n"
    "repeated_double: 1.234567898765\n"
    "repeated_double: 1.2345678987654\n"
    "repeated_double: 1.23456789876543\n"
    "repeated_double: 1.2e+100\n"
    "repeated_double: 1.23e+100\n"
    "repeated_double: 1.234e+100\n"
    "repeated_double: 1.2345e+100\n"
    "repeated_double: 1.23456e+100\n"
    "repeated_double: 1.234567e+100\n"
    "repeated_double: 1.2345678e+100\n"
    "repeated_double: 1.23456789e+100\n"
    "repeated_double: 1.234567898e+100\n"
    "repeated_double: 1.2345678987e+100\n"
    "repeated_double: 1.23456789876e+100\n"
    "repeated_double: 1.234567898765e+100\n"
    "repeated_double: 1.2345678987654e+100\n"
    "repeated_double: 1.23456789876543e+100\n",
    RemoveRedundantZeros(message.DebugString()));
}


TEST_F(TextFormatTest, AllowPartial) {
  unittest::TestRequired message;
  TextFormat::Parser parser;
  parser.AllowPartialMessage(true);
  EXPECT_TRUE(parser.ParseFromString("a: 1", &message));
  EXPECT_EQ(1, message.a());
  EXPECT_FALSE(message.has_b());
  EXPECT_FALSE(message.has_c());
}

TEST_F(TextFormatTest, ParseExotic) {
  unittest::TestAllTypes message;
  ASSERT_TRUE(TextFormat::ParseFromString(
    "repeated_int32: -1\n"
    "repeated_int32: -2147483648\n"
    "repeated_int64: -1\n"
    "repeated_int64: -9223372036854775808\n"
    "repeated_uint32: 4294967295\n"
    "repeated_uint32: 2147483648\n"
    "repeated_uint64: 18446744073709551615\n"
    "repeated_uint64: 9223372036854775808\n"
    "repeated_double: 123.0\n"
    "repeated_double: 123.5\n"
    "repeated_double: 0.125\n"
    "repeated_double: 1.23E17\n"
    "repeated_double: 1.235E+22\n"
    "repeated_double: 1.235e-18\n"
    "repeated_double: 123.456789\n"
    "repeated_double: inf\n"
    "repeated_double: Infinity\n"
    "repeated_double: -inf\n"
    "repeated_double: -Infinity\n"
    "repeated_double: nan\n"
    "repeated_double: NaN\n"
    "repeated_string: \"\\000\\001\\a\\b\\f\\n\\r\\t\\v\\\\\\'\\\"\"\n",
    &message));

  ASSERT_EQ(2, message.repeated_int32_size());
  EXPECT_EQ(-1, message.repeated_int32(0));
  // Note:  In C, a negative integer literal is actually the unary negation
  //   operator being applied to a positive integer literal, and 2147483648 is
  //   outside the range of int32.  However, it is not outside the range of
  //   uint32.  Confusingly, this means that everything works if we make the
  //   literal unsigned, even though we are negating it.
  EXPECT_EQ(-2147483648u, message.repeated_int32(1));

  ASSERT_EQ(2, message.repeated_int64_size());
  EXPECT_EQ(-1, message.repeated_int64(0));
  // Note:  In C, a negative integer literal is actually the unary negation
  //   operator being applied to a positive integer literal, and
  //   9223372036854775808 is outside the range of int64.  However, it is not
  //   outside the range of uint64.  Confusingly, this means that everything
  //   works if we make the literal unsigned, even though we are negating it.
  EXPECT_EQ(-GOOGLE_ULONGLONG(9223372036854775808), message.repeated_int64(1));

  ASSERT_EQ(2, message.repeated_uint32_size());
  EXPECT_EQ(4294967295u, message.repeated_uint32(0));
  EXPECT_EQ(2147483648u, message.repeated_uint32(1));

  ASSERT_EQ(2, message.repeated_uint64_size());
  EXPECT_EQ(GOOGLE_ULONGLONG(18446744073709551615), message.repeated_uint64(0));
  EXPECT_EQ(GOOGLE_ULONGLONG(9223372036854775808), message.repeated_uint64(1));

  ASSERT_EQ(13, message.repeated_double_size());
  EXPECT_EQ(123.0     , message.repeated_double(0));
  EXPECT_EQ(123.5     , message.repeated_double(1));
  EXPECT_EQ(0.125     , message.repeated_double(2));
  EXPECT_EQ(1.23E17   , message.repeated_double(3));
  EXPECT_EQ(1.235E22  , message.repeated_double(4));
  EXPECT_EQ(1.235E-18 , message.repeated_double(5));
  EXPECT_EQ(123.456789, message.repeated_double(6));
  EXPECT_EQ(message.repeated_double(7), numeric_limits<double>::infinity());
  EXPECT_EQ(message.repeated_double(8), numeric_limits<double>::infinity());
  EXPECT_EQ(message.repeated_double(9), -numeric_limits<double>::infinity());
  EXPECT_EQ(message.repeated_double(10), -numeric_limits<double>::infinity());
  EXPECT_TRUE(IsNaN(message.repeated_double(11)));
  EXPECT_TRUE(IsNaN(message.repeated_double(12)));

  // Note:  Since these string literals have \0's in them, we must explicitly
  //   pass their sizes to string's constructor.
  ASSERT_EQ(1, message.repeated_string_size());
  EXPECT_EQ(string("\000\001\a\b\f\n\r\t\v\\\'\"", 12),
            message.repeated_string(0));
}

class TextFormatParserTest : public testing::Test {
 protected:
  void ExpectFailure(const string& input, const string& message, int line,
                     int col) {
    unittest::TestAllTypes proto;
    ExpectFailure(input, message, line, col, &proto);
  }

  void ExpectFailure(const string& input, const string& message, int line,
                     int col, Message* proto) {
    TextFormat::Parser parser;
    MockErrorCollector error_collector;
    parser.RecordErrorsTo(&error_collector);
    EXPECT_FALSE(parser.ParseFromString(input, proto));
    EXPECT_EQ(SimpleItoa(line) + ":" + SimpleItoa(col) + ": " + message + "\n",
              error_collector.text_);
  }

  // An error collector which simply concatenates all its errors into a big
  // block of text which can be checked.
  class MockErrorCollector : public io::ErrorCollector {
   public:
    MockErrorCollector() {}
    ~MockErrorCollector() {}

    string text_;

    // implements ErrorCollector -------------------------------------
    void AddError(int line, int column, const string& message) {
      strings::SubstituteAndAppend(&text_, "$0:$1: $2\n",
                                   line + 1, column + 1, message);
    }
  };
};

TEST_F(TextFormatParserTest, InvalidToken) {
  ExpectFailure("optional_bool: true\n-5\n", "Expected identifier.",
                2, 1);

  ExpectFailure("optional_bool: true;\n", "Expected identifier.", 1, 20);
  ExpectFailure("\"some string\"", "Expected identifier.", 1, 1);
}

TEST_F(TextFormatParserTest, InvalidFieldName) {
  ExpectFailure(
      "invalid_field: somevalue\n",
      "Message type \"protobuf_unittest.TestAllTypes\" has no field named "
      "\"invalid_field\".",
      1, 14);
}

TEST_F(TextFormatParserTest, InvalidCapitalization) {
  // We require that group names be exactly as they appear in the .proto.
  ExpectFailure(
      "optionalgroup {\na: 15\n}\n",
      "Message type \"protobuf_unittest.TestAllTypes\" has no field named "
      "\"optionalgroup\".",
      1, 15);
  ExpectFailure(
      "OPTIONALgroup {\na: 15\n}\n",
      "Message type \"protobuf_unittest.TestAllTypes\" has no field named "
      "\"OPTIONALgroup\".",
      1, 15);
  ExpectFailure(
      "Optional_Double: 10.0\n",
      "Message type \"protobuf_unittest.TestAllTypes\" has no field named "
      "\"Optional_Double\".",
      1, 16);
}

TEST_F(TextFormatParserTest, InvalidFieldValues) {
  // Invalid values for a double/float field.
  ExpectFailure("optional_double: \"hello\"\n", "Expected double.", 1, 18);
  ExpectFailure("optional_double: true\n", "Expected double.", 1, 18);
  ExpectFailure("optional_double: !\n", "Expected double.", 1, 18);
  ExpectFailure("optional_double {\n  \n}\n", "Expected \":\", found \"{\".",
                1, 17);

  // Invalid values for a signed integer field.
  ExpectFailure("optional_int32: \"hello\"\n", "Expected integer.", 1, 17);
  ExpectFailure("optional_int32: true\n", "Expected integer.", 1, 17);
  ExpectFailure("optional_int32: 4.5\n", "Expected integer.", 1, 17);
  ExpectFailure("optional_int32: !\n", "Expected integer.", 1, 17);
  ExpectFailure("optional_int32 {\n \n}\n", "Expected \":\", found \"{\".",
                1, 16);
  ExpectFailure("optional_int32: 0x80000000\n",
                "Integer out of range.", 1, 17);
  ExpectFailure("optional_int32: -0x80000001\n",
                "Integer out of range.", 1, 18);
  ExpectFailure("optional_int64: 0x8000000000000000\n",
                "Integer out of range.", 1, 17);
  ExpectFailure("optional_int64: -0x8000000000000001\n",
                "Integer out of range.", 1, 18);

  // Invalid values for an unsigned integer field.
  ExpectFailure("optional_uint64: \"hello\"\n", "Expected integer.", 1, 18);
  ExpectFailure("optional_uint64: true\n", "Expected integer.", 1, 18);
  ExpectFailure("optional_uint64: 4.5\n", "Expected integer.", 1, 18);
  ExpectFailure("optional_uint64: -5\n", "Expected integer.", 1, 18);
  ExpectFailure("optional_uint64: !\n", "Expected integer.", 1, 18);
  ExpectFailure("optional_uint64 {\n \n}\n", "Expected \":\", found \"{\".",
                1, 17);
  ExpectFailure("optional_uint32: 0x100000000\n",
                "Integer out of range.", 1, 18);
  ExpectFailure("optional_uint64: 0x10000000000000000\n",
                "Integer out of range.", 1, 18);

  // Invalid values for a boolean field.
  ExpectFailure("optional_bool: \"hello\"\n", "Expected identifier.", 1, 16);
  ExpectFailure("optional_bool: 5\n", "Expected identifier.", 1, 16);
  ExpectFailure("optional_bool: -7.5\n", "Expected identifier.", 1, 16);
  ExpectFailure("optional_bool: !\n", "Expected identifier.", 1, 16);

  ExpectFailure(
      "optional_bool: meh\n",
      "Invalid value for boolean field \"optional_bool\". Value: \"meh\".",
      2, 1);

  ExpectFailure("optional_bool {\n \n}\n", "Expected \":\", found \"{\".",
                1, 15);

  // Invalid values for a string field.
  ExpectFailure("optional_string: true\n", "Expected string.", 1, 18);
  ExpectFailure("optional_string: 5\n", "Expected string.", 1, 18);
  ExpectFailure("optional_string: -7.5\n", "Expected string.", 1, 18);
  ExpectFailure("optional_string: !\n", "Expected string.", 1, 18);
  ExpectFailure("optional_string {\n \n}\n", "Expected \":\", found \"{\".",
                1, 17);

  // Invalid values for an enumeration field.
  ExpectFailure("optional_nested_enum: \"hello\"\n", "Expected identifier.",
                1, 23);

  ExpectFailure("optional_nested_enum: 5\n", "Expected identifier.", 1, 23);
  ExpectFailure("optional_nested_enum: -7.5\n", "Expected identifier.", 1, 23);
  ExpectFailure("optional_nested_enum: !\n", "Expected identifier.", 1, 23);

  ExpectFailure(
      "optional_nested_enum: grah\n",
      "Unknown enumeration value of \"grah\" for field "
      "\"optional_nested_enum\".", 2, 1);

 ExpectFailure(
      "optional_nested_enum {\n \n}\n",
      "Expected \":\", found \"{\".", 1, 22);
}

TEST_F(TextFormatParserTest, MessageDelimeters) {
  // Non-matching delimeters.
  ExpectFailure("OptionalGroup <\n \n}\n", "Expected \">\", found \"}\".",
                3, 1);

  // Invalid delimeters.
  ExpectFailure("OptionalGroup [\n \n]\n", "Expected \"{\", found \"[\".",
                1, 15);

  // Unending message.
  ExpectFailure("optional_nested_message {\n \nbb: 118\n",
                "Expected identifier.",
                4, 1);
}

TEST_F(TextFormatParserTest, UnknownExtension) {
  // Non-matching delimeters.
  ExpectFailure("[blahblah]: 123",
                "Extension \"blahblah\" is not defined or is not an "
                "extension of \"protobuf_unittest.TestAllTypes\".",
                1, 11);
}

TEST_F(TextFormatParserTest, MissingRequired) {
  unittest::TestRequired message;
  ExpectFailure("a: 1",
                "Message missing required fields: b, c",
                0, 1, &message);
}

TEST_F(TextFormatParserTest, ParseDuplicateRequired) {
  unittest::TestRequired message;
  ExpectFailure("a: 1 b: 2 c: 3 a: 1",
                "Non-repeated field \"a\" is specified multiple times.",
                1, 17, &message);
}

TEST_F(TextFormatParserTest, ParseDuplicateOptional) {
  unittest::ForeignMessage message;
  ExpectFailure("c: 1 c: 2",
                "Non-repeated field \"c\" is specified multiple times.",
                1, 7, &message);
}

TEST_F(TextFormatParserTest, MergeDuplicateRequired) {
  unittest::TestRequired message;
  TextFormat::Parser parser;
  EXPECT_TRUE(parser.MergeFromString("a: 1 b: 2 c: 3 a: 4", &message));
  EXPECT_EQ(4, message.a());
}

TEST_F(TextFormatParserTest, MergeDuplicateOptional) {
  unittest::ForeignMessage message;
  TextFormat::Parser parser;
  EXPECT_TRUE(parser.MergeFromString("c: 1 c: 2", &message));
  EXPECT_EQ(2, message.c());
}

TEST_F(TextFormatParserTest, PrintErrorsToStderr) {
  vector<string> errors;

  {
    ScopedMemoryLog log;
    unittest::TestAllTypes proto;
    EXPECT_FALSE(TextFormat::ParseFromString("no_such_field: 1", &proto));
    errors = log.GetMessages(ERROR);
  }

  ASSERT_EQ(1, errors.size());
  EXPECT_EQ("Error parsing text-format protobuf_unittest.TestAllTypes: "
            "1:14: Message type \"protobuf_unittest.TestAllTypes\" has no field "
            "named \"no_such_field\".",
            errors[0]);
}

TEST_F(TextFormatParserTest, FailsOnTokenizationError) {
  vector<string> errors;

  {
    ScopedMemoryLog log;
    unittest::TestAllTypes proto;
    EXPECT_FALSE(TextFormat::ParseFromString("\020", &proto));
    errors = log.GetMessages(ERROR);
  }

  ASSERT_EQ(1, errors.size());
  EXPECT_EQ("Error parsing text-format protobuf_unittest.TestAllTypes: "
            "1:1: Invalid control characters encountered in text.",
            errors[0]);
}


class TextFormatMessageSetTest : public testing::Test {
 protected:
  static const char proto_debug_string_[];
};
const char TextFormatMessageSetTest::proto_debug_string_[] =
"message_set {\n"
"  [protobuf_unittest.TestMessageSetExtension1] {\n"
"    i: 23\n"
"  }\n"
"  [protobuf_unittest.TestMessageSetExtension2] {\n"
"    str: \"foo\"\n"
"  }\n"
"}\n";


TEST_F(TextFormatMessageSetTest, Serialize) {
  protobuf_unittest::TestMessageSetContainer proto;
  protobuf_unittest::TestMessageSetExtension1* item_a =
    proto.mutable_message_set()->MutableExtension(
      protobuf_unittest::TestMessageSetExtension1::message_set_extension);
  item_a->set_i(23);
  protobuf_unittest::TestMessageSetExtension2* item_b =
    proto.mutable_message_set()->MutableExtension(
      protobuf_unittest::TestMessageSetExtension2::message_set_extension);
  item_b->set_str("foo");
  EXPECT_EQ(proto_debug_string_, proto.DebugString());
}

TEST_F(TextFormatMessageSetTest, Deserialize) {
  protobuf_unittest::TestMessageSetContainer proto;
  ASSERT_TRUE(TextFormat::ParseFromString(proto_debug_string_, &proto));
  EXPECT_EQ(23, proto.message_set().GetExtension(
    protobuf_unittest::TestMessageSetExtension1::message_set_extension).i());
  EXPECT_EQ("foo", proto.message_set().GetExtension(
    protobuf_unittest::TestMessageSetExtension2::message_set_extension).str());

  // Ensure that these are the only entries present.
  vector<const FieldDescriptor*> descriptors;
  proto.message_set().GetReflection()->ListFields(
    proto.message_set(), &descriptors);
  EXPECT_EQ(2, descriptors.size());
}

}  // namespace text_format_unittest
}  // namespace protobuf

}  // namespace google