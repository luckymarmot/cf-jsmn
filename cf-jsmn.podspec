#
# Be sure to run `pod lib lint cf-jsmn.podspec' to ensure this is a
# valid spec before submitting.
#

Pod::Spec.new do |s|
  s.name             = "cf-jsmn"
  s.version          = "0.1.0"
  s.summary          = "CF (OS X and iOS) version of jsmn"
  s.description      = <<-DESC
    A Core Foundation compatible version of the awesome lightweight JSON parser
    "jsmn", with support for unicode strings.
  DESC
  s.homepage         = "https://github.com/luckymarmot/cf-jsmn"
  s.license          = 'MIT'
  s.author           = { "Micha Mazaheri" => "micha@mazaheri.me" }
  s.source           = { :git => "https://github.com/luckymarmot/cf-jsmn.git", :tag => s.version.to_s }
  s.social_media_url = 'https://twitter.com/luckymarmot'

  s.ios.deployment_target = '7.0'
  s.osx.deployment_target = '10.9'
  s.requires_arc = false

  s.source_files = 'src/*'
  s.public_header_files = 'src/*.h'
end
