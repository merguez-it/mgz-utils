# Generated by jeweler
# DO NOT EDIT THIS FILE DIRECTLY
# Instead, edit Jeweler::Tasks in Rakefile, and run 'rake gemspec'
# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = %q{digest-whirlpool}
  s.version = "1.0.1"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = [%q{Akinori MUSHA}]
  s.date = %q{2011-08-10}
  s.description = %q{This is a Digest module implementing the Whirlpool hashing algorithm.
The size of a Whirlpool hash value is 512 bits.
}
  s.email = %q{knu@idaemons.org}
  s.extensions = [%q{ext/digest/whirlpool/extconf.rb}]
  s.files = [
    ".document",
    ".gitignore",
    "LICENSE",
    "README.rdoc",
    "Rakefile",
    "VERSION",
    "ext/digest/whirlpool/depend",
    "ext/digest/whirlpool/extconf.rb",
    "ext/digest/whirlpool/whirlpool-algorithm.c",
    "ext/digest/whirlpool/whirlpool-algorithm.h",
    "ext/digest/whirlpool/whirlpool-constants.h",
    "ext/digest/whirlpool/whirlpool-portability.h",
    "ext/digest/whirlpool/whirlpool.c",
    "test/test_digest-whirlpool.rb"
  ]
  s.homepage = %q{http://github.com/knu/ruby-digest-extra}
  s.require_paths = [%q{lib}]
  s.rubygems_version = %q{1.8.7}
  s.summary = %q{A Digest module implementing the Whirlpool hashing algorithm}

  if s.respond_to? :specification_version then
    s.specification_version = 3

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
    else
    end
  else
  end
end

