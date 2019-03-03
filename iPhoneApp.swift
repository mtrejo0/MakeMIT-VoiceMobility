//
//  ViewController.swift
//  MakeMIT
//
//  Created by Enrique Avina on 3/2/19.
//  Copyright © 2019 MakeSquad. All rights reserved.
//

import UIKit
import UIKit
import Speech
import Alamofire

class ViewController: UIViewController, SFSpeechRecognizerDelegate {
    
    @IBOutlet weak var commandLabel: UILabel!
    @IBOutlet weak var microphoneButton: UIButton!
    
    private let speechRecognizer = SFSpeechRecognizer(locale: Locale.init(identifier: "en-US"))
    private var recognitionRequest: SFSpeechAudioBufferRecognitionRequest?
    private var recognitionTask: SFSpeechRecognitionTask?
    private let audioEngine = AVAudioEngine()
    
    private var commands: [String] = []
    
    override func viewDidLoad() {
        super.viewDidLoad()

        microphoneButton.isEnabled = false
        speechRecognizer?.delegate = self
        
        SFSpeechRecognizer.requestAuthorization { (authStatus) in
            var isButtonEnabled = false
            
            switch authStatus {
            case .authorized:
                isButtonEnabled = true
            case .denied:
                isButtonEnabled = false
                print("User denied access to speech recognition")
            case .restricted:
                isButtonEnabled = false
                print("Speech recognition restricted on this device")
            case .notDetermined:
                isButtonEnabled = false
                print("Speech recognition not yet authorized")
            }
            
            OperationQueue.main.addOperation() {
                self.microphoneButton.isEnabled = isButtonEnabled
            }
        }
    }
    

    @IBAction func record(_ sender: UIButton) {
        if audioEngine.isRunning {
            audioEngine.stop()
            recognitionRequest?.endAudio()
            microphoneButton.isEnabled = false
            
            microphoneButton.setTitle("Start Recording", for: .normal)
        } else {
            startRecording()
            microphoneButton.setTitle("Stop Recording", for: .normal)
        }
    }
    
    func recurse_send_post_request(commands: [String]){
        if commands.count < 1{
            return
        } else{
            let currElement: Int = convertToInt(command: commands[0])
            var newCommands: [String] = commands
            newCommands.remove(at: 0)
            send_post_request(command: currElement) {
                self.recurse_send_post_request(commands: newCommands)
            }
        }
    }
    
    func send_post_request(command: Int, completion: @escaping (()->())){
        let data: [String: Int] = ["command": command]
        Alamofire.request("http://608dev.net/sandbox/sc/eavina/makemit/req.py", method: .post, parameters: data, encoding: URLEncoding.default).responseString { (response) in
            completion()
        }
    }
    
    func convertToInt(command: String) -> Int {
        switch command.lowercased() {
        case "stop":
            return 0
        case "forward":
            return 1
        case "forwards":
            return 1
        case "back":
            return 2
        case "backward":
            return 2
        case "backwards":
            return 2
        case "left":
            return 3
        case "right":
            return 4
        default:
            return -1
        }
    }
    
//    func recurseQueue(res: [String]){
//        if res.Count < 1:
//            self.send_post_request(command: resInt, completion: { })
//
//    }
    
    func startRecording() {
        
        if recognitionTask != nil {
            recognitionTask?.cancel()
            recognitionTask = nil
        }
        
        let audioSession = AVAudioSession.sharedInstance()
        do {
            try audioSession.setCategory(AVAudioSessionCategoryRecord)
            try audioSession.setMode(AVAudioSessionModeMeasurement)
            try audioSession.setActive(true, with: .notifyOthersOnDeactivation)
        } catch {
            print("audioSession properties weren't set because of an error.")
        }
        
        recognitionRequest = SFSpeechAudioBufferRecognitionRequest()
        
        let inputNode = audioEngine.inputNode
        
        guard let recognitionRequest = recognitionRequest else {
            fatalError("Unable to create an SFSpeechAudioBufferRecognitionRequest object")
        }
        
        recognitionRequest.shouldReportPartialResults = true
        
        recognitionTask = speechRecognizer?.recognitionTask(with: recognitionRequest, resultHandler: { (result, error) in
            
            var isFinal = false
            
            if result != nil {
                self.commandLabel.text = result?.bestTranscription.formattedString
                isFinal = (result?.isFinal)!
            }
            
            if error != nil || isFinal {
                self.audioEngine.stop()
                inputNode.removeTap(onBus: 0)
                
                
                let res:[String] = (result?.bestTranscription.formattedString)?.components(separatedBy: " ") ?? []
                
                // 0 stop 1 forward 2 back 3 left 4 right
                
                OperationQueue.main.addOperation {
                    self.recurse_send_post_request(commands: res)
                }
                
                self.recognitionRequest = nil
                self.recognitionTask = nil
                
                self.microphoneButton.isEnabled = true
            }
        })
        
        let recordingFormat = inputNode.outputFormat(forBus: 0)
        inputNode.installTap(onBus: 0, bufferSize: 1024, format: recordingFormat) { (buffer, when) in
            self.recognitionRequest?.append(buffer)
        }
        
        audioEngine.prepare()
        
        do {
            try audioEngine.start()
        } catch {
            print("audioEngine couldn't start because of an error.")
        }
        
        commandLabel.text = "Say something, I'm listening!"
        
    }
    
    func speechRecognizer(_ speechRecognizer: SFSpeechRecognizer, availabilityDidChange available: Bool) {
        if available {
            microphoneButton.isEnabled = true
        } else {
            microphoneButton.isEnabled = false
        }
    }
    
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}
