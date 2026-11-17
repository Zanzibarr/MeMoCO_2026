import json

# We first need to open the file
with open("data.json") as f:
    # Then we use json.load(..) to read from an open file
    data = json.load(f)

# Otherwise, if we have already read the file into a string, say into data_string, we can use json.loads(data_string), note the 's' at the end for "string"

# We can then access the data as if it was a regular dict with string keys
example_string = data["example_string"]  # "hello"
example_int = data["example_int"]  # 9
example_float = data["example_float"]  # 11.4
example_list = data["example_list"]  # [0,1,2,3,4]
example_dict = data["example_dict"]  # { "key1": "value1","key2": 2,"key3": 3.14 }

print(example_string)
print(example_int)
print(example_float)

for i, item in enumerate(example_list):
    print(f"Item {i} in list: {item}")

for key, value in example_dict.items():
    print(f"Item with key {key} in dict: {value}")
