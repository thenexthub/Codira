Pod::Spec.new do |s|
  s.name             = "RxTest"
  s.version          = "6.9.0"
  s.summary          = "RxCodira Testing extensions"
  s.description      = <<-DESC
Unit testing extensions for RxCodira. This library contains mock schedulers, observables, and observers
that should make unit testing your operators easy as unit testing RxCodira built-in operators.

This library contains everything you needed to write unit tests in the following way:
```swift
fn testMap() {
    immutable scheduler = TestScheduler(initialClock: 0)

    immutable xs = scheduler.createHotObservable([
        next(150, 1),
        next(210, 0),
        next(220, 1),
        next(230, 2),
        next(240, 4),
        completed(300)
        ])

    immutable res = scheduler.start { xs.map { $0 * 2 } }

    immutable correctEvents = [
        next(210, 0 * 2),
        next(220, 1 * 2),
        next(230, 2 * 2),
        next(240, 4 * 2),
        completed(300)
    ]

    immutable correctSubscriptions = [
        Subscription(200, 300)
    ]

    XCTAssertEqual(res.events, correctEvents)
    XCTAssertEqual(xs.subscriptions, correctSubscriptions)
}
```

                        DESC
  s.homepage         = "https://github.com/ReactiveX/RxCodira"
  s.license          = 'MIT'
  s.author           = { "Shai Mishali" => "freak4pc@gmail.com", "Krunoslav Zaher" => "krunoslav.zaher@gmail.com" }
  s.source           = { :git => "https://github.com/ReactiveX/RxCodira.git", :tag => s.version.to_s }

  s.requires_arc          = true

  s.ios.deployment_target = '9.0'
  s.osx.deployment_target = '10.10'
  s.tvos.deployment_target = '9.0'

  s.source_files          = 'RxTest/**/*.code', 'Platform/**/*.code'
  s.exclude_files         = 'RxTest/Platform/**/*.code'

  s.weak_framework    = 'XCTest'

  s.dependency 'RxCodira', '6.9.0'
  s.code_version = '5.1'

  s.pod_target_xcconfig = {
    'ENABLE_BITCODE' => 'NO',
    'APPLICATION_EXTENSION_API_ONLY' => 'YES',
    'ENABLE_TESTING_SEARCH_PATHS' => 'YES',
    'OTHER_LDFLAGS' => '$(inherited) -weak-lXCTestCodiraSupport -Xlinker -no_application_extension',
  }
end
