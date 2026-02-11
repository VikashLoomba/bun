//
//  BunRuntime.swift
//  Swift wrapper for the Bun iOS embedding API
//

import Foundation

/// Wraps the Bun C API for use in Swift
class BunRuntime {
    
    /// Current state of the runtime
    enum State {
        case idle
        case running
        case terminated(exitCode: Int32)
    }
    
    private(set) var state: State = .idle
    
    private var stdinPipe: Pipe?
    private var stdoutPipe: Pipe?
    private var outputHandler: ((String) -> Void)?
    private var exitHandler: ((Int32) -> Void)?
    
    /// Callback storage (must be retained)
    private var exitCallbackPtr: UnsafeMutablePointer<(Int32) -> Void>?
    
    init() {}
    
    deinit {
        exitCallbackPtr?.deallocate()
    }
    
    /// Run a JavaScript file
    /// - Parameters:
    ///   - scriptPath: Path to the .js or .ts file
    ///   - workingDirectory: Working directory for the script
    ///   - onOutput: Called when Bun writes to stdout
    ///   - onExit: Called when Bun exits
    func run(
        script scriptPath: String,
        workingDirectory: String? = nil,
        onOutput: @escaping (String) -> Void,
        onExit: @escaping (Int32) -> Void
    ) {
        guard case .idle = state else {
            onExit(-1)
            return
        }
        
        state = .running
        outputHandler = onOutput
        exitHandler = onExit
        
        // Create pipes for I/O
        stdinPipe = Pipe()
        stdoutPipe = Pipe()
        
        // Set up output reading
        stdoutPipe?.fileHandleForReading.readabilityHandler = { [weak self] handle in
            let data = handle.availableData
            guard !data.isEmpty else { return }
            
            if let text = String(data: data, encoding: .utf8) {
                DispatchQueue.main.async {
                    self?.outputHandler?(text)
                }
            }
        }
        
        // Prepare arguments
        let cwd = workingDirectory ?? FileManager.default.currentDirectoryPath
        let args = [cwd, "run", scriptPath]
        
        // Convert to C strings
        var cArgs = args.map { strdup($0) }
        cArgs.append(nil)
        
        // Create exit callback
        // Note: We need to use a C function pointer, so we store context globally
        let exitCallback: @convention(c) (UInt32) -> Void = { code in
            // This is called from Bun's thread - dispatch to main
            DispatchQueue.main.async {
                NotificationCenter.default.post(
                    name: .bunDidExit,
                    object: nil,
                    userInfo: ["exitCode": Int32(code)]
                )
            }
        }
        
        // Register for exit notification
        NotificationCenter.default.addObserver(
            forName: .bunDidExit,
            object: nil,
            queue: .main
        ) { [weak self] notification in
            guard let self = self else { return }
            let code = notification.userInfo?["exitCode"] as? Int32 ?? -1
            self.handleExit(code: code)
        }
        
        // Start Bun
        let result = bun_start(
            Int32(args.count),
            &cArgs,
            stdinPipe!.fileHandleForReading.fileDescriptor,
            stdoutPipe!.fileHandleForWriting.fileDescriptor,
            -1,  // stderr same as stdout
            exitCallback
        )
        
        // Clean up C strings
        for ptr in cArgs where ptr != nil {
            free(ptr)
        }
        
        if result != 0 {
            state = .terminated(exitCode: -1)
            onExit(-1)
        }
    }
    
    /// Write data to Bun's stdin
    func write(_ text: String) {
        guard case .running = state else { return }
        
        if let data = text.data(using: .utf8) {
            stdinPipe?.fileHandleForWriting.write(data)
        }
    }
    
    /// Close stdin (signals EOF to Bun)
    func closeInput() {
        try? stdinPipe?.fileHandleForWriting.close()
    }
    
    private func handleExit(code: Int32) {
        state = .terminated(exitCode: code)
        
        // Clean up pipes
        stdoutPipe?.fileHandleForReading.readabilityHandler = nil
        try? stdinPipe?.fileHandleForWriting.close()
        try? stdoutPipe?.fileHandleForReading.close()
        
        stdinPipe = nil
        stdoutPipe = nil
        
        exitHandler?(code)
        exitHandler = nil
        outputHandler = nil
    }
}

// MARK: - Notification

extension Notification.Name {
    static let bunDidExit = Notification.Name("BunDidExit")
}

// MARK: - Convenience

extension BunRuntime {
    /// Evaluate JavaScript code directly
    func eval(
        _ code: String,
        workingDirectory: String? = nil,
        onOutput: @escaping (String) -> Void,
        onExit: @escaping (Int32) -> Void
    ) {
        guard case .idle = state else {
            onExit(-1)
            return
        }
        
        state = .running
        outputHandler = onOutput
        exitHandler = onExit
        
        stdinPipe = Pipe()
        stdoutPipe = Pipe()
        
        stdoutPipe?.fileHandleForReading.readabilityHandler = { [weak self] handle in
            let data = handle.availableData
            guard !data.isEmpty else { return }
            if let text = String(data: data, encoding: .utf8) {
                DispatchQueue.main.async {
                    self?.outputHandler?(text)
                }
            }
        }
        
        let cwd = workingDirectory ?? FileManager.default.currentDirectoryPath
        
        let exitCallback: @convention(c) (UInt32) -> Void = { code in
            DispatchQueue.main.async {
                NotificationCenter.default.post(
                    name: .bunDidExit,
                    object: nil,
                    userInfo: ["exitCode": Int32(code)]
                )
            }
        }
        
        NotificationCenter.default.addObserver(
            forName: .bunDidExit,
            object: nil,
            queue: .main
        ) { [weak self] notification in
            guard let self = self else { return }
            let exitCode = notification.userInfo?["exitCode"] as? Int32 ?? -1
            self.handleExit(code: exitCode)
        }
        
        let result = bun_eval(
            cwd,
            code,
            stdinPipe!.fileHandleForReading.fileDescriptor,
            stdoutPipe!.fileHandleForWriting.fileDescriptor,
            -1,
            exitCallback
        )
        
        if result != 0 {
            state = .terminated(exitCode: -1)
            onExit(-1)
        }
    }
}
