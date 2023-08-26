import tldextract
import sys

if __name__ == "__main__":
    with open(sys.argv[1]) as f:
        for line in f:
            line = line.strip()
            if not line:continue
            rd = tldextract.extract(line).registered_domain
            print(rd)
        # end for
    # end with
# end if
