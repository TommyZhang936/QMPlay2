/* Copyright (c) 2013 Dropbox, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

#pragma once

#include <QByteArray>

#include <vector>
#include <memory>
#include <map>

enum JsonParse {
	STANDARD, COMMENTS
};

class JsonValue;

class Json final {
public:
	// Types
	enum Type {
		NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT
	};

	// Array and object typedefs
	using array  = std::vector<Json>;
	using object = std::map<QByteArray, Json>;

	// Constructors for the various types of JSON value.
	Json() noexcept;                // NUL
	Json(std::nullptr_t) noexcept;  // NUL
	Json(double value);             // NUMBER
	Json(int value);                // NUMBER
	Json(bool value);               // BOOL
	Json(const QByteArray &value);  // STRING
	Json(QByteArray &&value);       // STRING
	Json(const char * value);       // STRING
	Json(const array &values);      // ARRAY
	Json(array &&values);           // ARRAY
	Json(const object &values);     // OBJECT
	Json(object &&values);          // OBJECT

	// Implicit constructor: anything with a to_json() function.
	template <class T, class = decltype(&T::to_json)>
	Json(const T & t) : Json(t.to_json()) {}

	// Implicit constructor: map-like objects (std::map, std::unordered_map, etc)
	template <class M, typename std::enable_if<
		std::is_constructible<QByteArray, typename M::key_type>::value
		&& std::is_constructible<Json, typename M::mapped_type>::value,
			int>::type = 0>
	Json(const M & m) : Json(object(m.begin(), m.end())) {}

	// Implicit constructor: vector-like objects (std::list, std::vector, std::set, etc)
	template <class V, typename std::enable_if<
		std::is_constructible<Json, typename V::value_type>::value,
			int>::type = 0>
	Json(const V & v) : Json(array(v.begin(), v.end())) {}

	// This prevents Json(some_pointer) from accidentally producing a bool. Use
	// Json(bool(some_pointer)) if that behavior is desired.
	Json(void *) = delete;

	// Accessors
	Type type() const;

	bool is_null()   const { return type() == NUL; }
	bool is_number() const { return type() == NUMBER; }
	bool is_bool()   const { return type() == BOOL; }
	bool is_string() const { return type() == STRING; }
	bool is_array()  const { return type() == ARRAY; }
	bool is_object() const { return type() == OBJECT; }

	// Return the enclosed value if this is a number, 0 otherwise. Note that json11 does not
	// distinguish between integer and non-integer numbers - number_value() and int_value()
	// can both be applied to a NUMBER-typed object.
	double number_value() const;
	int int_value() const;

	// Return the enclosed value if this is a boolean, false otherwise.
	bool bool_value() const;
	// Return the enclosed string if this is a string, "" otherwise.
	const QByteArray &string_value() const;
	// Return the enclosed std::vector if this is an array, or an empty vector otherwise.
	const array &array_items() const;
	// Return the enclosed std::map if this is an object, or an empty map otherwise.
	const object &object_items() const;

	// Return a reference to arr[i] if this is an array, Json() otherwise.
	const Json & operator[](int i) const;
	// Return a reference to obj[key] if this is an object, Json() otherwise.
	const Json & operator[](const QByteArray &key) const;

	// Serialize.
	void dump(QByteArray &out) const;
	QByteArray dump() const {
		QByteArray out;
		dump(out);
		return out;
	}

	// Parse.
	static inline Json parse(const QByteArray & in,
					  JsonParse strategy = JsonParse::STANDARD) {
		QByteArray err;
		return parse(in, err, strategy);
	}
	// Parse. If parse fails, return Json() and assign an error message to err.
	static Json parse(const QByteArray & in,
					  QByteArray & err,
					  JsonParse strategy = JsonParse::STANDARD);

	// Parse multiple objects, concatenated or separated by whitespace
	static std::vector<Json> parse_multi(
		const QByteArray & in,
		int & parser_stop_pos,
		QByteArray & err,
		JsonParse strategy = JsonParse::STANDARD);
	static inline std::vector<Json> parse_multi(
		const QByteArray & in,
		QByteArray & err,
		JsonParse strategy = JsonParse::STANDARD) {
		int parser_stop_pos;
		return parse_multi(in, parser_stop_pos, err, strategy);
	}

	bool operator== (const Json &rhs) const;
	bool operator<  (const Json &rhs) const;
	bool operator!= (const Json &rhs) const { return !(*this == rhs); }
	bool operator<= (const Json &rhs) const { return !(rhs < *this); }
	bool operator>  (const Json &rhs) const { return  (rhs < *this); }
	bool operator>= (const Json &rhs) const { return !(*this < rhs); }

private:
	std::shared_ptr<JsonValue> m_ptr;
};

// Internal class hierarchy - JsonValue objects are not exposed to users of this API.
class JsonValue {
protected:
	friend class Json;
	friend class JsonInt;
	friend class JsonDouble;
	virtual Json::Type type() const = 0;
	virtual bool equals(const JsonValue * other) const = 0;
	virtual bool less(const JsonValue * other) const = 0;
	virtual void dump(QByteArray &out) const = 0;
	virtual double number_value() const;
	virtual int int_value() const;
	virtual bool bool_value() const;
	virtual const QByteArray &string_value() const;
	virtual const Json::array &array_items() const;
	virtual const Json &operator[](int i) const;
	virtual const Json::object &object_items() const;
	virtual const Json &operator[](const QByteArray &key) const;
	virtual ~JsonValue() = default;
};
