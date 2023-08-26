import tldextract
import sys
from collections import Counter
if __name__ == "__main__":
    if (len(sys.argv) != 2):
        printf("Usage: {} <file-name>".format(sys.argv[1]))
        exit(1)
    # end if
    data = list()
    with open(sys.argv[1]) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            data.append(tldextract.extract(line).suffix)
        # end for
    # end with
    data = Counter(data).most_common()
    for x in data:
        print(x[1], x[0])
    # end for
# end main
