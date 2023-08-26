#include <libctld.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cunittest.h>

void assert_expect(ctld_ctx * ctld, int is_private, char * record, char * fqdn,
                   char * rd, char * domain, char * suffix){
    ctld_result * result;
    result = ctld_parse(ctld, record, is_private);
    //fprintf(stdout, "%s\n%s\n%s\n%s\n", result->fqdn, result->registered_domain, result->domain, result->suffix);
    ASSERT_NE_NULL(result);
    if (fqdn)
        ASSERT_EQ_STR_NOCASE(result->fqdn, fqdn);
    else
        ASSERT_NULL(result->fqdn);

    if (rd)
        ASSERT_EQ_STR_NOCASE(result->registered_domain, rd);
    else
        ASSERT_NULL(result->registered_domain);
    
    if (domain)
        ASSERT_EQ_STR_NOCASE(result->domain, domain);
    else
        ASSERT_NULL(result->domain);

    if (suffix)
        ASSERT_EQ_STR_NOCASE(result->suffix, suffix);
    else
        ASSERT_NULL(result->suffix);
}

int test(){
    ctld_ctx * ctx = ctld_parse_file("psl.dat");
    ASSERT_NE_NULL(ctx);
    // now start testing stuff
    assert_expect(ctx, 0, "google.com", "google.com", "google.com", "google", "com");
    assert_expect(ctx, 1, "google.com", "google.com", "google.com", "google", "com");
    assert_expect(ctx, 1, "gooGle.cOm", "google.com", "google.com", "google", "com");
    assert_expect(ctx, 1, "www.google.com", "www.google.com", "google.com", "google", "com");
    assert_expect(ctx, 1, "www.theregister.co.uk", "www.theregister.co.uk", "theregister.co.uk", "theregister", "co.uk");
    assert_expect(ctx, 1, "gmail.com", "gmail.com", "gmail.com", "gmail", "com");
    assert_expect(ctx, 1, "media.forums.theregister.co.uk", "media.forums.theregister.co.uk", "theregister.co.uk", "theregister", "co.uk");
    assert_expect(ctx, 1, "www.www.com", "www.www.com", "www.com", "www", "com");
    assert_expect(ctx, 1, "www.com", "www.com", "www.com", "www", "com");
    assert_expect(ctx, 1, "com", NULL, NULL, NULL, "com");
    assert_expect(ctx, 1, "co.uk", NULL, NULL, NULL, "co.uk");
    assert_expect(ctx, 0, "example.ck", NULL, NULL, NULL, "example.ck");
    assert_expect(ctx, 1, "example.ck", NULL, NULL, NULL, "example.ck");
    assert_expect(ctx, 1, "www.example.ck", "www.example.ck", "www.example.ck", "www", "example.ck");
    assert_expect(ctx, 1, "sub.www.example.ck", "sub.www.example.ck", "www.example.ck", "www", "example.ck");
    assert_expect(ctx, 1, "www.ck", "www.ck", "www.ck", "www", "ck");
    assert_expect(ctx, 0, "www.ck", "www.ck", "www.ck", "www", "ck");
    assert_expect(ctx, 1, "nes.buskerud.no", NULL, NULL, NULL, "nes.buskerud.no");
    assert_expect(ctx, 0, "nes.buskerud.no", NULL, NULL, NULL, "nes.buskerud.no");
    assert_expect(ctx, 1, "buskerud.no", "buskerud.no", "buskerud.no", "buskerud", "no");
    assert_expect(ctx, 1, "internalunlikelyhostname.info", "internalunlikelyhostname.info", "internalunlikelyhostname.info", "internalunlikelyhostname", "info");
    assert_expect(ctx, 1, "xn--h1alffa9f.xn--p1ai", "xn--h1alffa9f.xn--p1ai", "xn--h1alffa9f.xn--p1ai", "xn--h1alffa9f", "xn--p1ai");
    assert_expect(ctx, 1, "xn--tub-1m9d15sfkkhsifsbqygyujjrw602gk4li5qqk98aca0w.google.com", "xn--tub-1m9d15sfkkhsifsbqygyujjrw602gk4li5qqk98aca0w.google.com",
                  "google.com", "google", "com");
    assert_expect(ctx, 1, "xn--tub-1m9d15sfkkhsifsbqygyujjrw60.google.com", "xn--tub-1m9d15sfkkhsifsbqygyujjrw60.google.com",
                  "google.com", "google", "com");
    assert_expect(ctx, 1, "xn--zckzap6140b352by.blog.so-net.xn--wcvs22d.hk", "xn--zckzap6140b352by.blog.so-net.xn--wcvs22d.hk",
                  "so-net.xn--wcvs22d.hk", "so-net", "xn--wcvs22d.hk");
    assert_expect(ctx, 1, "xn--&.so-net.com", "xn--&.so-net.com", "so-net.com", "so-net", "com");
    assert_expect(ctx, 1, "xn--ß‌꫶ᢥ.com", "xn--ß‌꫶ᢥ.com", "xn--ß‌꫶ᢥ.com", "xn--ß‌꫶ᢥ", "com");
    assert_expect(ctx, 1, "xn--zckzap6140b352by.blog.so-net.教育.hk", "xn--zckzap6140b352by.blog.so-net.教育.hk",
                  "so-net.教育.hk", "so-net", "教育.hk");
    assert_expect(ctx, 1, "xn--gieen46ers-73a.de", "xn--gieen46ers-73a.de", "xn--gieen46ers-73a.de", "xn--gieen46ers-73a", "de");
    assert_expect(ctx, 1, "www.cgs.act.edu.au", "www.cgs.act.edu.au", "cgs.act.edu.au", "cgs", "act.edu.au");
    assert_expect(ctx, 0, "www.metp.net.cn", "www.metp.net.cn", "metp.net.cn", "metp", "net.cn");
    assert_expect(ctx, 1, "foo.blogspot.com", "foo.blogspot.com", "foo.blogspot.com", "foo", "blogspot.com");
    assert_expect(ctx, 1, "s3.ap-south-1.amazonaws.com", NULL, NULL, NULL, "s3.ap-south-1.amazonaws.com");
    assert_expect(ctx, 1, "the-quick-brown-fox.ap-south-1.amazonaws.com", "the-quick-brown-fox.ap-south-1.amazonaws.com",
                  "amazonaws.com", "amazonaws", "com");
    assert_expect(ctx, 1, "s3.cn-north-1.amazonaws.com.cn", NULL, NULL, NULL, "s3.cn-north-1.amazonaws.com.cn");
    assert_expect(ctx, 1, "amazonaws.com.cn", "amazonaws.com.cn", "amazonaws.com.cn", "amazonaws", "com.cn");
    
    // won't pass this test since we don't convert the input, user can convert it before passing to the library
    //assert_expect(ctx, 0, "angelinablog。com.de", "angelinablog。com.de", "com.de", "com", "de");
    //assert_expect(ctx, 1, "angelinablog。com.de", "angelinablog。com.de", "angelinablog。com.de", "angelinablog", "com.de");
    
    //free the memory (but it's not necessary!)
    ctld_free(ctx);
    return 0;
}

int main(int argc, char ** argv){
    assert(test() == 0);
    printf("*** All tests passed successfully!\n");
    return 0;
}



