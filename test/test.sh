#!/bin/bash

# see if there is any possible error using ctld binary file
FAIL="Test failed...."
test $(echo "subdomain.google.com"| ./bin/ctld --rd) == 'google.com' || echo $FAIL
test $(echo "https://freefire.xn--npth-5q5a1g.vn/app" | ./bin/ctld --rd) == 'xn--npth-5q5a1g.vn' || echo $FAIL
test $(echo "http://nạpthẻ.vn/app" | ./bin/ctld --rd) == 'xn--npth-5q5a1g.vn' || echo $FAIL
test $(echo "http://xn--npth-5q5a1g.nạpthẻ.vn/app" | ./bin/ctld --rd) == 'xn--npth-5q5a1g.vn' || echo $FAIL

test $(echo "nạpthẻ.vn" | ./bin/ctld --rd) == 'xn--npth-5q5a1g.vn' || echo $FAIL
test $(echo "xn--npth-5q5a1g.nạpthẻ.vn" | ./bin/ctld --rd) == 'xn--npth-5q5a1g.vn' || echo $FAIL

test $(echo "xn--npth-5q5a1g.nạpthẻ.vn" | ./bin/ctld --domain) == 'xn--npth-5q5a1g' || echo $FAIL
test $(echo "https://freefire.xn--npth-5q5a1g.vn/app" | ./bin/ctld --domain) == 'xn--npth-5q5a1g' || echo $FAIL
test $(echo "https://test.vic.edu.au/app" | ./bin/ctld --domain) == 'test' || echo $FAIL