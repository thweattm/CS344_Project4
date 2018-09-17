main: keygen encServer encClient decServer decClient

keygen: keygen.c
	gcc keygen.c -o keygen -g

encServer: otp_enc_d.c
	gcc otp_enc_d.c -o otp_enc_d -g

encClient: otp_enc.c
	gcc otp_enc.c -o otp_enc -g

decServer: otp_dec_d.c
	gcc otp_dec_d.c -o otp_dec_d -g

decClient: otp_dec.c
	gcc otp_dec.c -o otp_dec -g

clean:
	rm -f otp_enc_d
	rm -f otp_enc
	rm -f otp_dec_d
	rm -f otp_dec
	rm -f keygen
