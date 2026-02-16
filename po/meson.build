gtext = find_program('xgettext', required: get_option('nls'))
if not gtext.found()
  subdir_done()
endif

i18n = import('i18n')

i18n.gettext(
  'exiv2',
  preset: 'glib',
  args: [
    '--files-from=@0@/POTFILES.in'.format(meson.current_source_dir()),
    '--copyright-holder="Exiv2 authors"',
    '--msgid-bugs-address=https://github.com/Exiv2/exiv2/issues',
  ],
)
