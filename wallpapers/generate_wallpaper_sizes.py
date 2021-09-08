#!/usr/bin/env python3

import logging
from pathlib import Path
from itertools import chain

try:
	from PIL import Image
except ImportError:
	logging.critical("Please install the python PIL library.")
	logging.critical("e.g.: python3 -m pip install PIL")
	exit()

sizes = {
	'horizontal': (
		(5120, 2880), (3840, 2160), (3200, 2000), (3200, 1800),
		(2560, 1600), (2560, 1440), (1920, 1200), (1920, 1080),
		(1680, 1050), (1600, 1200), (1440, 900), (1366, 768),
		(1280, 1024), (1280, 800), (1024, 768), (440, 247)
		),
	'vertical': ((720, 1440), (360, 720), (1080, 1920))
	}

templates = {
	'horizontal': ('base_size.png', 'base_size.jpg'),
	'vertical':  ('vertical_base_size.png', 'vertical_base_size.jpg')
	}

for orientation in ('horizontal', 'vertical'):
	for file in chain(*map(Path().rglob, templates[orientation])):
		image = Image.open(file)
		image_path = Path(file)
		base_dir, extension = image_path.parent, image_path.suffix
		base_width, base_height = image.size
		for width, height in sizes[orientation]:
			logging.info(f'Generating {width}x{height}')
			if width/height > base_width/base_height:
				crop = int(base_height - height/(width/base_width))//2
				box = (0, crop, base_width, base_height-crop)
			elif width/height < base_width/base_height:
				crop = int(base_width - width/(height/base_height))//2
				box = (crop, 0, base_width-crop, base_height)
			else: box = None
			resized_image = image.resize((width, height), Image.LANCZOS, box)
			resized_image.save(base_dir / f'{width}x{height}{extension}',
					  quality=90, optimize=True, subsampling=1)
