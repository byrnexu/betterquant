# WARNING
* 🔥 To facilitate locating the problem, there are some logs containing sensitive information in the system, which have a certain impact on performance (several microseconds) or security.
* 🔥 At present, the api key is stored in the database and not encrypted, which is very insecure. In the future, an encryption plug-in will be added to encrypt and store it in the database, or you can modify GetApiInfo to read and decrypt it yourself. Of course, if your apiKey is bound to an IP and the transfer function is turned off, There shouldn't be much of a safety issue.
