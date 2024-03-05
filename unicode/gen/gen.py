import argparse
import unicode_data
import scripts

DEFAULT_UNICODE_VERSION = "15.1.0"

def main():
	argparser = argparse.ArgumentParser()
	argparser.add_argument("--version", default=DEFAULT_UNICODE_VERSION)
	args = argparser.parse_args()
	print("Unicode version:", args.version)
	unicode_data.generate(version=args.version)
	scripts.generate(version=args.version)

if __name__ == "__main__":
	main()
