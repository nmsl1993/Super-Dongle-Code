all: nanopb_pb2.py plugin_pb2.py

%_pb2.py: %.proto
	protoc --python_out=. $<

clean:
	rm -f nanopb_pb2.py plugin_pb2.py
