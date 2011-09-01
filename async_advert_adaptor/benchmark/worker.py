from optparse import OptionParser
import saga

def main():
	parser = OptionParser()
	parser.add_option("-u", "--url", dest="adverturl", help="The advert server url", metavar="URL");
	parser.add_option("-i", "--id", dest="workerid", help="The workers ID", metavar="ID");
	
	(options, args) = parser.parse_args()

	saga.advert.directory(options.adverturl)
        
def run_loop():
	print("hello")


if __name__ == "__main__":
	main()