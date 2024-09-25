/**
 * @file main.cpp
 * @author Barnabas Mulu Boka, Amanuel Hayele Tsegay, Habteab Teame, Teklit
 * Amanuel Ghebremichael
 *
 * This certificate is downloaded from Chrome.
 *
 * 1. Go to web site ipify.org.
 * 2. Right click on the symbol for web site information and choose
 *    information about web site certificate.
 * 3. Open tab security chain and select certificate next to the end of chain.
 *    This will be the certificate of a Certificate Authority that has issued
 *    the certificate for ipify.org, in this case Sectigo RSA Domain Validation
 *    Secure Server CA.
 * 4. Copy the certificate (X.509) to file in PEM (Base-64-encodet ASCII)
 *    format.
 * 5. Open file from GitBash using the vim text editor and issue commands
 *    :%norm I"        to add " at start of each line, then
 *    :%norm A\n"      to add \n" at end of each line, then
 *    :wq              to save file and quit.
 * 6. Copy content to some source file
 *    const char root_cert[] = <the copied pem cert>;
 */

#ifndef __NewsBBC__
#define __NewsBBC__

// Certificate of issuer Sectigo RSA Domain Validation Secure Server CA
const char NewsBBC_Certificate_group35[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDAjCCAomgAwIBAgINAe5fIpVCSQX5AZGo3DAKBggqhkjOPQQDAzBQMSQwIgYD\n"
"VQQLExtHbG9iYWxTaWduIEVDQyBSb290IENBIC0gUjUxEzARBgNVBAoTCkdsb2Jh\n"
"bFNpZ24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMTgxMTIxMDAwMDAwWhcNMjgx\n"
"MTIxMDAwMDAwWjBQMQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBu\n"
"di1zYTEmMCQGA1UEAxMdR2xvYmFsU2lnbiBFQ0MgT1YgU1NMIENBIDIwMTgwdjAQ\n"
"BgcqhkjOPQIBBgUrgQQAIgNiAATDoRGNZSPhluG7q6bQA11PTe0ZD/xx44QlFam1\n"
"BM4eLeN+wfgwalsbkjzARCM9si/fnQeKNtKAlgNmNOHTmV3VfwGbocj6+22HVWZu\n"
"VeX/VeIGoWh1u7Lja/NDE7RsXaCjggEpMIIBJTAOBgNVHQ8BAf8EBAMCAYYwEgYD\n"
"VR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUWHuOdSr+YYCqkEABrtboB0ZuP0gw\n"
"HwYDVR0jBBgwFoAUPeYpSJvqB8ohREom3m7e0oPQn1kwPgYIKwYBBQUHAQEEMjAw\n"
"MC4GCCsGAQUFBzABhiJodHRwOi8vb2NzcDIuZ2xvYmFsc2lnbi5jb20vcm9vdHI1\n"
"MDYGA1UdHwQvMC0wK6ApoCeGJWh0dHA6Ly9jcmwuZ2xvYmFsc2lnbi5jb20vcm9v\n"
"dC1yNS5jcmwwRwYDVR0gBEAwPjA8BgRVHSAAMDQwMgYIKwYBBQUHAgEWJmh0dHBz\n"
"Oi8vd3d3Lmdsb2JhbHNpZ24uY29tL3JlcG9zaXRvcnkvMAoGCCqGSM49BAMDA2cA\n"
"MGQCMC4lzZGQw5mpNZBmztq8huxKf9/tRUJ5yLI4q6YU+i2fjF2FRBNA64EBmljA\n"
"7dkSOwIwL9qYB0APhsLmV0LhknrzHZVvtqzg7NQaIV18BEIDZQgK3gjxYzADjHSH\n"
"5uk4mCdW\n"
"-----END CERTIFICATE-----\n";


#endif //__NewsBBC__
