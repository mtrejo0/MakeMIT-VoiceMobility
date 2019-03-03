import speech_recognition as sr
import string
import requests
import json
needed={'right':4,'left':3,'forward':1,'forwards':1,'backwards':2,'backward':2,'back':2,'stop':0}
condition=True
while(condition):
    keywords=[]
    r=sr.Recognizer()
    mic = sr.Microphone()
    a=''
    with mic as source:
        #r.adjust_for_ambient_noise(source)
        audio=r.listen(source,phrase_time_limit=3)
    try:
        x=(r.recognize_google(audio))
        y=x.split(" ")
        # print(y)
        for i in y:
            if i.lower()in needed:
                if i.lower() == 'backwards':
                    keywords.append(2)
                elif i.lower()=='forwards':
                    keywords.append(1)
                if i.lower() == 'back':
                    keywords.append(2)
                else:
                    keywords.append(needed[i.lower()])
        print(keywords)
        myurl = "http://608dev.net/sandbox/sc/eavina/makemit/req.py"
        if len(keywords)==0:
            print("No Words were said.")
        else:
            for i in keywords:
                a = requests.post(url=myurl,data={'command':i})
                print(a.status_code)
                print(a.content)
        a=input("Y for again N\n")
        if a.lower()=='n':
            condition = False
    except sr.UnknownValueError:
        print("Error")
    except sr.RequestError as e:
        print("Could not request results from Google Speech Recognition service; {0}".format(e))
