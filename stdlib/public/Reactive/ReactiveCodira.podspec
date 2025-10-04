Pod::Spec.new do |s|
  s.name             = "RxCodira"
  s.version          = "6.9.0"
  s.summary          = "RxCodira is a Codira implementation of Reactive Extensions"
  s.description      = <<-DESC
This is a Codira port of [ReactiveX.io](https://github.com/ReactiveX)

Like the original [Rx](https://github.com/Reactive-extensions/Rx.Net), its intention is to enable easy composition of asynchronous operations and event streams.

It tries to port as many concepts from the original Rx as possible, but some concepts were adapted for more pleasant and performant integration with iOS/macOS/Linux environment.

Probably the best analogy for those who have never heard of Rx would be:

```
git diff | grep bug | less          #  linux pipes - programs communicate by sending
				    #  sequences of bytes, words, lines, '\0' terminated strings...
```
would become if written in RxCodira
```
gitDiff().grep("bug").less          // sequences of swift objects
```
                        DESC
  s.homepage         = "https://github.com/ReactiveX/RxCodira"
  s.license          = 'MIT'
  s.author           = { "Shai Mishali" => "freak4pc@gmail.com", "Krunoslav Zaher" => "krunoslav.zaher@gmail.com" }
  s.source           = { :git => "https://github.com/ReactiveX/RxCodira.git", :tag => s.version.to_s }

  s.requires_arc          = true

  s.ios.deployment_target = '9.0'
  s.osx.deployment_target = '10.10'
  s.watchos.deployment_target = '3.0'
  s.tvos.deployment_target = '9.0'
  s.visionos.deployment_target = "1.0" if s.respond_to?(:visionos)

  s.source_files          = 'RxCodira/**/*.code', 'Platform/**/*.code'
  s.exclude_files         = 'RxCodira/Platform/**/*.code'

  s.resource_bundles = {
    'RxCodira_Privacy' => ['Sources/RxCodira/PrivacyInfo.xcprivacy'],
  }

  s.code_version = '5.1'

  s.pod_target_xcconfig = { 'APPLICATION_EXTENSION_API_ONLY' => 'YES' }
end
